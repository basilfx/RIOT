/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Minimal HTTP/1.1 server on top of sock_tcp, using
 *              picohttpparser to parse requests
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mjson.h"
#include "net/sock/tcp.h"
#include "picohttpparser.h"
#include "sched.h"
#include "schedstatistics.h"
#include "shell.h"
#include "thread.h"
#include "time_units.h"
#include "walltime.h"

#include "blob/index.html.h"
#include "blob/assets/riot-logo.svg.h"
#include "blob/assets/github-logo.svg.h"
#include "blob/assets/theme-icon.svg.h"

#ifndef SERVER_PORT
#  define SERVER_PORT             (8080)
#endif

/* chosen to match the 6 parallel connections a browser opens per origin
 * (Chrome and Firefox both cap there). Deeper absorbs more of a page load
 * without a refusal, but reclaiming the queue after every slot is filled by
 * an idle connection is strictly serial, so it is also, in the worst case,
 * proportionally slower to recover from; see README's Limitations section */
#define SOCK_QUEUE_LEN          (6)
#define REQUEST_BUF_SIZE        (2048)
#define MAX_HEADERS             (16)
#define STATS_BUF_SIZE          (1024)
#define DRAIN_BUF_SIZE          (64)

/* the sock API documents its timeouts in microseconds, but the GNRC
 * implementation forwards them to gnrc_tcp_recv(), which expects
 * milliseconds. The timeouts below are therefore expressed in milliseconds
 * and scaled for the stacks that honour the documented unit */
#if defined(MODULE_LWIP_SOCK_TCP)
#   define SOCK_TIMEOUT_MS(ms)  ((uint32_t)(ms) * US_PER_MS)
#  define NETWORK_STACK         "lwip"
#else
#  define SOCK_TIMEOUT_MS(ms)   ((uint32_t)(ms))
#  define NETWORK_STACK         "gnrc"
#endif

/* the IP version is a Makefile-level choice (IPV6=1), reflected here in
 * which of the two stacks' IPv6 module ends up pulled in */
#if defined(MODULE_GNRC_IPV6) || defined(MODULE_LWIP_IPV6)
#  define LOCAL_EP_ANY          SOCK_IPV6_EP_ANY
#else
#  define LOCAL_EP_ANY          SOCK_IPV4_EP_ANY
#endif

/* a client that opens a connection but never completes a request must not be
 * able to occupy a sock indefinitely. This thread is the only one accepting
 * connections, so it can attend to one client at a time, while the stack
 * meanwhile completes the handshake of further clients on the remaining
 * socks of the listening queue. Those connections are only reclaimed once
 * this thread accepts them, and nothing else ever times them out. Without an
 * upper bound on how long a single client is waited for, a handful of idle
 * connections, such as the spare ones a browser keeps open for reuse, is
 * enough to occupy every sock in the queue permanently, after which the
 * stack answers every further connection attempt with a reset */
#define REQUEST_TIMEOUT_MS      (2000)

/* how long the peer is given to close the connection first, see
 * _drain_and_close() */
#define LINGER_TIMEOUT_MS       (500)

/* an upper bound on the number of reads spent waiting for the peer to close,
 * so that a client that keeps sending cannot hold on to the connection */
#define LINGER_MAX_READS        (8)

#define LOGO_PATH               "/assets/riot-logo.svg"
#define GITHUB_LOGO_PATH        "/assets/github-logo.svg"
#define THEME_ICON_PATH         "/assets/theme-icon.svg"
#define STATS_PATH              "/api/stats"
#define VERSION_PATH            "/api/version"

static char _server_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];

/* the listening queue outlives every call, so it is kept off the server
 * thread's stack */
static sock_tcp_t _sock_queue[SOCK_QUEUE_LEN];

/* only the single server thread ever updates or reads this counter, so a
 * plain increment is correct. Revisit this if the server ever grows a
 * second thread that also serves requests */
static uint32_t _requests;

/* the amount of data a single sock_tcp_write() accepts is bounded by the
 * current send window, so the caller has to loop until everything was handed
 * over to the stack */
static int _write_all(sock_tcp_t *sock, const void *data, size_t len)
{
    const uint8_t *pos = data;

    while (len) {
        ssize_t res = sock_tcp_write(sock, pos, len);

        if (res < 0) {
            return (int)res;
        }

        pos += res;
        len -= (size_t)res;
    }

    return 0;
}

static int _send_response(sock_tcp_t *sock, const char *status,
                           const char *content_type,
                           const void *body, size_t body_len)
{
    char header[192];
    int header_len = snprintf(header, sizeof(header),
                               "HTTP/1.1 %s\r\n"
                               "Content-Type: %s\r\n"
                               "Content-Length: %u\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               status, content_type, (unsigned)body_len);
    int res = _write_all(sock, header, (size_t)header_len);

    if (res == 0 && body_len) {
        res = _write_all(sock, body, body_len);
    }

    if (res < 0) {
        printf("Error writing response: %s\n", strerror(-res));
    }

    return res;
}

/* waits for the peer to close the connection before closing the local end
 * point. A close initiated by this side leaves the local end point in the
 * TIME_WAIT state, and as gnrc_tcp_close() blocks until that state is left,
 * the listening socks would be unavailable for the duration of it */
static void _drain_and_close(sock_tcp_t *sock)
{
    static char drain_buf[DRAIN_BUF_SIZE];

    for (unsigned i = 0; i < LINGER_MAX_READS; i++) {
        if (sock_tcp_read(sock, drain_buf, sizeof(drain_buf),
                          SOCK_TIMEOUT_MS(LINGER_TIMEOUT_MS)) <= 0) {
            break;
        }
    }

    sock_tcp_disconnect(sock);
}

/* appends a single, already-formatted thread entry to fb, unless doing so
 * would leave no room for the closing "]}" that _build_stats_json() still
 * has to append once the loop over threads is done. Used instead of just
 * relying on mjson_printf()'s own fixed-buffer printer truncating on
 * overflow, since that would cut a thread entry off midway and leave the
 * JSON invalid, rather than stopping cleanly before it */
static bool _append_thread_entry(struct mjson_fixedbuf *fb, const char *entry,
                                  size_t entry_len)
{
    if ((size_t)(fb->size - fb->len) < entry_len + 2) {
        return false;
    }

    mjson_print_buf(mjson_print_fixed_buf, fb, entry, (int)entry_len);
    return true;
}

/* counts how many of the listening queue's slots currently hold an
 * established connection. sock_tcp_get_remote() reports -ENOTCONN for a
 * slot that does not, on both gnrc and lwip, so this works the same way
 * regardless of which stack the queue was built against */
static unsigned _count_active_connections(void)
{
    unsigned active = 0;

    for (unsigned i = 0; i < SOCK_QUEUE_LEN; i++) {
        sock_tcp_ep_t ep;

        if (sock_tcp_get_remote(&_sock_queue[i], &ep) == 0) {
            active++;
        }
    }

    return active;
}

/* builds a JSON object of the request counter, uptime and the current
 * thread and scheduler statistics, truncating gracefully if the static
 * buffer is too small to hold all threads rather than overflowing it */
static size_t _build_stats_json(char *buf, size_t buf_len)
{
    struct mjson_fixedbuf fb = { .ptr = buf, .size = (int)buf_len, .len = 0 };
    bool first = true;

    mjson_printf(mjson_print_fixed_buf, &fb, "{%Q:%u", "requests",
                 (int)_requests);

    mjson_printf(mjson_print_fixed_buf, &fb, ",%Q:%u", "uptime_sec",
                 (int)walltime_uptime(false));

    mjson_printf(mjson_print_fixed_buf, &fb, ",%Q:%u", "connections_active",
                 _count_active_connections());
    mjson_printf(mjson_print_fixed_buf, &fb, ",%Q:%u", "connections_max",
                 (unsigned)SOCK_QUEUE_LEN);

    mjson_printf(mjson_print_fixed_buf, &fb, ",%Q:[", "threads");

    for (kernel_pid_t pid = KERNEL_PID_FIRST; pid <= KERNEL_PID_LAST; pid++) {
        thread_t *thread = thread_get(pid);

        if (thread == NULL) {
            continue;
        }

        const char *name = thread_get_name(thread);
        /* built in its own scratch buffer first, since the entry must not
         * be split across the room check in _append_thread_entry() below */
        char entry[160];
        int entry_len = mjson_snprintf(entry, sizeof(entry),
                                        "%s{%Q:%d,%Q:%Q,%Q:%Q,%Q:%u,"
                                        "%Q:%u,%Q:%u,%Q:%u,%Q:%u}",
                                        first ? "" : ",",
                                        "pid", (int)pid,
                                        "name", name ? name : "?",
                                        "state", thread_state_to_string(
                                                     thread_get_status(thread)),
                                        "priority", (int)thread_get_priority(thread),
                                        "stack_size", (int)thread_get_stacksize(thread),
                                        "stack_used",
                                        (int)(thread_get_stacksize(thread) -
                                              thread_measure_stack_free(thread)),
                                        "runtime_us",
                                        (int)sched_pidlist[pid].runtime_us,
                                        "switches",
                                        (int)sched_pidlist[pid].schedules);

        if (!_append_thread_entry(&fb, entry, (size_t)entry_len)) {
            break;
        }

        first = false;
    }

    mjson_printf(mjson_print_fixed_buf, &fb, "]}");

    return (size_t)fb.len;
}

static void _handle_client(sock_tcp_t *sock)
{
    /* static to keep the thread's stack usage low. Connections are queued by
     * the stack, but they are accepted one after another, so a single buffer
     * suffices */
    static char buf[REQUEST_BUF_SIZE];
    size_t buf_len = 0;
    size_t prev_buf_len;

    while (1) {
        if (buf_len == sizeof(buf)) {
            /* the request headers did not fit the buffer */
            _send_response(sock, "431 Request Header Fields Too Large",
                            "text/plain", NULL, 0);
            return;
        }

        int res = sock_tcp_read(sock, buf + buf_len, sizeof(buf) - buf_len,
                                 SOCK_TIMEOUT_MS(REQUEST_TIMEOUT_MS));
        if (res <= 0) {
            /* the peer closed the connection, or it did not complete its
             * request in time, see REQUEST_TIMEOUT_MS */
            return;
        }

        prev_buf_len = buf_len;
        buf_len += res;

        const char *method, *path;
        size_t method_len, path_len;
        int minor_version;
        /* static for the same reason as buf, this keeps the parser scratch
         * space off the thread's stack */
        static struct phr_header headers[MAX_HEADERS];
        size_t num_headers = MAX_HEADERS;

        int parsed = phr_parse_request(buf, buf_len, &method, &method_len,
                                        &path, &path_len, &minor_version,
                                        headers, &num_headers, prev_buf_len);
        if (parsed == -1) {
            _send_response(sock, "400 Bad Request", "text/plain", NULL, 0);
            return;
        }
        else if (parsed == -2) {
            /* the request is incomplete, read more data */
            continue;
        }

        /* counts every request that reaches a response, including the
         * 400 and 404 branches below, not just the ones that resolve to a
         * route */
        _requests++;

        printf("Request: %.*s %.*s\n", (int)method_len, method,
               (int)path_len, path);

        if (!(method_len == 3 && strncmp(method, "GET", 3) == 0)) {
            _send_response(sock, "404 Not Found", "text/plain", NULL, 0);
            return;
        }

        if (path_len == 1 && path[0] == '/') {
            _send_response(sock, "200 OK", "text/html",
                            index_html, index_html_len);
        }
        else if (path_len == sizeof(LOGO_PATH) - 1 &&
                 strncmp(path, LOGO_PATH, path_len) == 0) {
            _send_response(sock, "200 OK", "image/svg+xml",
                            riot_logo_svg, riot_logo_svg_len);
        }
        else if (path_len == sizeof(GITHUB_LOGO_PATH) - 1 &&
                 strncmp(path, GITHUB_LOGO_PATH, path_len) == 0) {
            _send_response(sock, "200 OK", "image/svg+xml",
                            github_logo_svg, github_logo_svg_len);
        }
        else if (path_len == sizeof(THEME_ICON_PATH) - 1 &&
                 strncmp(path, THEME_ICON_PATH, path_len) == 0) {
            _send_response(sock, "200 OK", "image/svg+xml",
                            theme_icon_svg, theme_icon_svg_len);
        }
        else if (path_len == sizeof(STATS_PATH) - 1 &&
                 strncmp(path, STATS_PATH, path_len) == 0) {
            static char stats_buf[STATS_BUF_SIZE];
            size_t stats_len = _build_stats_json(stats_buf, sizeof(stats_buf));

            _send_response(sock, "200 OK", "application/json",
                            stats_buf, stats_len);
        }
        else if (path_len == sizeof(VERSION_PATH) - 1 &&
                 strncmp(path, VERSION_PATH, path_len) == 0) {
            char version_buf[128];
            int version_len = mjson_snprintf(version_buf, sizeof(version_buf),
                                              "{%Q:%Q,%Q:%Q,%Q:%Q}",
                                              "version", RIOT_VERSION,
                                              "board", RIOT_BOARD,
                                              "network_stack", NETWORK_STACK);

            _send_response(sock, "200 OK", "application/json",
                            version_buf, (size_t)version_len);
        }
        else {
            _send_response(sock, "404 Not Found", "text/plain", NULL, 0);
        }
        return;
    }
}

static void *_server_thread(void *arg)
{
    (void)arg;
    sock_tcp_queue_t queue;
    sock_tcp_ep_t local = LOCAL_EP_ANY;

    local.port = SERVER_PORT;

    if (sock_tcp_listen(&queue, &local, _sock_queue, SOCK_QUEUE_LEN, 0) < 0) {
        puts("Error creating listening queue");
        return NULL;
    }

    printf("HTTP server listening on port %u\n", (unsigned)SERVER_PORT);

    while (1) {
        sock_tcp_t *sock;

        if (sock_tcp_accept(&queue, &sock, SOCK_NO_TIMEOUT) < 0) {
            puts("Error accepting new sock");
            continue;
        }

        _handle_client(sock);
        _drain_and_close(sock);
    }

    sock_tcp_stop_listen(&queue);
    return NULL;
}

int main(void)
{
    puts("RIOT HTTP server example application");

    thread_create(_server_stack, sizeof(_server_stack),
                  THREAD_PRIORITY_MAIN - 1, 0,
                  _server_thread, NULL, "http_server");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
/** @} */
