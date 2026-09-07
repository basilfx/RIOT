/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "event.h"
#include "event/timeout.h"
#include "kernel_defines.h"
#include "log.h"
#include "net/dhcpv4.h"
#include "net/dhcpv4/client.h"
#include "net/sock.h"
#include "net/sock/udp.h"
#include "random.h"
#include "thread.h"
#include "timex.h"
#if IS_USED(MODULE_ZTIMER)
#include "ztimer.h"
#else
#include "xtimer.h"
#include "xtimer/implementation.h"
#endif

#define ENABLE_DEBUG 0
#include "debug.h"

#include "_dhcpv4.h"

/**
 * @brief   Client states, per RFC 2131 section 4.4
 */
enum {
    STATE_INIT,
    STATE_SELECTING,
    STATE_REQUESTING,
    STATE_BOUND,
    STATE_RENEWING,
    STATE_REBINDING,
};

/**
 * @brief   Fields of an OFFER/ACK/NAK relevant to this client, extracted from
 *          its options
 */
typedef struct {
    uint8_t type;
    bool has_server_id;
    bool has_dns;
    ipv4_addr_t server_id;
    ipv4_addr_t dns;
    uint32_t t1;
    uint32_t t2;
    dhcpv4_client_lease_t lease;
} _parsed_msg_t;

static uint8_t send_buf[DHCPV4_CLIENT_BUFLEN];
static uint8_t recv_buf[DHCPV4_CLIENT_BUFLEN];
static uint8_t hwaddr[16];
static uint8_t hwaddr_len;
static uint8_t htype;
static sock_udp_t sock;
static sock_udp_ep_t local = { .family = AF_INET, .port = DHCPV4_CLIENT_PORT };
static sock_udp_ep_t remote_broadcast = { .family = AF_INET,
                                          .port = DHCPV4_SERVER_PORT,
                                          .addr = { .ipv4 = { 255, 255, 255, 255 } } };
static sock_udp_ep_t remote_server = { .family = AF_INET,
                                       .port = DHCPV4_SERVER_PORT };
static event_queue_t *event_queue;
static event_timeout_t restart_timeout, t1_timeout, t2_timeout;
static uint32_t xid;
static uint32_t transaction_start_sec;
static uint8_t state = STATE_INIT;
static uint16_t netif;
static ipv4_addr_t offered_addr;
static ipv4_addr_t server_id;
static dhcpv4_client_lease_t lease;
static uint32_t lease_start_sec;
static uint32_t t1_sec, t2_sec;

static void _init(event_t *event);
static void _request(event_t *event);
static void _renew(event_t *event);
static void _rebind(event_t *event);
static void _decline(event_t *event);

static event_t init_event = { .handler = _init };
static event_t request_event = { .handler = _request };
static event_t renew_event = { .handler = _renew };
static event_t rebind_event = { .handler = _rebind };
static event_t decline_event = { .handler = _decline };

static void _set_event_timeout_ms(event_timeout_t *timeout, event_t *event,
                                  uint32_t delay_ms)
{
#if IS_USED(MODULE_EVENT_TIMEOUT_ZTIMER)
    event_timeout_ztimer_init(timeout, ZTIMER_MSEC, event_queue, event);
    event_timeout_set(timeout, delay_ms);
#else
    event_timeout_init(timeout, event_queue, event);
    event_timeout_set(timeout, delay_ms * US_PER_MS);
#endif
}

static void _set_event_timeout_sec(event_timeout_t *timeout, event_t *event,
                                   uint32_t delay_sec)
{
#if IS_USED(MODULE_EVENT_TIMEOUT_ZTIMER)
    event_timeout_ztimer_init(timeout, ZTIMER_SEC, event_queue, event);
    event_timeout_set(timeout, delay_sec);
#else
    event_timeout_init(timeout, event_queue, event);
    /* use xtimer_set64 instead of event_timeout_set to prevent overflows */
    xtimer_set64(&timeout->timer, ((uint64_t)delay_sec) * US_PER_SEC);
#endif
}

static void _clear_event_timeout(event_timeout_t *timeout)
{
    event_timeout_clear(timeout);
}

static inline uint32_t _now_sec(void)
{
#if IS_USED(MODULE_ZTIMER)
    return (uint32_t)ztimer_now(ZTIMER_SEC);
#else
    return (uint32_t)(xtimer_now_usec64() / US_PER_SEC);
#endif
}

static void _schedule_restart(uint32_t delay_ms)
{
    _clear_event_timeout(&t1_timeout);
    _clear_event_timeout(&t2_timeout);
    _set_event_timeout_ms(&restart_timeout, &init_event, delay_ms);
}

static uint32_t _next_rt_ms(uint32_t rt_ms)
{
    uint32_t next_ms = rt_ms * 2;
    /* RFC 2131, section 4.1: randomize by -1s to +1s */
    int32_t jitter_ms = (int32_t)random_uint32_range(0, 2 * MS_PER_SEC) -
                        (int32_t)MS_PER_SEC;

    if (next_ms > CONFIG_DHCPV4_CLIENT_MAX_RT_MS) {
        next_ms = CONFIG_DHCPV4_CLIENT_MAX_RT_MS;
    }
    if (((int32_t)next_ms + jitter_ms) > 0) {
        next_ms = (uint32_t)((int32_t)next_ms + jitter_ms);
    }
    return next_ms;
}

static void _flush_stale_replies(void)
{
    int res;

    while ((res = sock_udp_recv(&sock, recv_buf, sizeof(recv_buf), 0,
                                NULL)) >= 0) {
        DEBUG("DHCPv4 client: discarding %d stale bytes\n", res);
    }
}

static void _generate_xid(void)
{
    xid = random_uint32();
}

static void _set_secs(dhcpv4_msg_t *msg)
{
    uint32_t elapsed = _now_sec() - transaction_start_sec;

    msg->secs = byteorder_htons((elapsed > 0xffff) ? 0xffff : (uint16_t)elapsed);
}

/**
 * @brief   Fills in the fixed part of a DHCPv4 message
 *
 * @param[in] ciaddr    The client's own address, if it already has one
 *                      configured and can therefore receive a unicast reply
 *                      (RENEWING/REBINDING/DECLINE). NULL otherwise
 *                      (INIT/SELECTING/REQUESTING), which also sets
 *                      @ref DHCPV4_FLAG_BROADCAST.
 */
static void _build_msg_hdr(dhcpv4_msg_t *msg, const ipv4_addr_t *ciaddr)
{
    memset(msg, 0, sizeof(*msg));
    msg->op = DHCPV4_OP_BOOTREQUEST;
    msg->htype = htype;
    msg->hlen = hwaddr_len;
    msg->xid = byteorder_htonl(xid);
    _set_secs(msg);
    msg->flags = byteorder_htons((ciaddr == NULL) ? DHCPV4_FLAG_BROADCAST : 0);
    if (ciaddr != NULL) {
        msg->ciaddr = *ciaddr;
    }
    memcpy(msg->chaddr, hwaddr, hwaddr_len);
    msg->magic_cookie = byteorder_htonl(DHCPV4_MAGIC_COOKIE);
}

static size_t _add_param_req_list(uint8_t *buf)
{
    uint8_t prl[] = {
        DHCPV4_OPT_SUBNET_MASK,
        DHCPV4_OPT_ROUTER,
#if IS_USED(MODULE_SOCK_DNS)
        DHCPV4_OPT_DNS_SERVER,
#endif
        DHCPV4_OPT_LEASE_TIME,
        DHCPV4_OPT_RENEWAL_TIME,
        DHCPV4_OPT_REBINDING_TIME,
        DHCPV4_OPT_CLASSLESS_ROUTES,
    };

    return dhcpv4_opt_add(buf, DHCPV4_OPT_PARAM_REQ_LIST, prl, sizeof(prl));
}

static size_t _build_discover(void)
{
    dhcpv4_msg_t *msg = (dhcpv4_msg_t *)send_buf;
    uint8_t msg_type = DHCPV4_MSG_DISCOVER;
    size_t len = sizeof(dhcpv4_msg_t);

    _build_msg_hdr(msg, NULL);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_MSG_TYPE, &msg_type, 1);
    len += _add_param_req_list(&send_buf[len]);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_END, NULL, 0);
    return len;
}

static size_t _build_request_selecting(void)
{
    dhcpv4_msg_t *msg = (dhcpv4_msg_t *)send_buf;
    uint8_t msg_type = DHCPV4_MSG_REQUEST;
    size_t len = sizeof(dhcpv4_msg_t);

    _build_msg_hdr(msg, NULL);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_MSG_TYPE, &msg_type, 1);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_REQUESTED_IP,
                          &offered_addr, sizeof(offered_addr));
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_SERVER_ID,
                          &server_id, sizeof(server_id));
    len += _add_param_req_list(&send_buf[len]);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_END, NULL, 0);
    return len;
}

/**
 * @brief   Builds a RENEWING/REBINDING DHCPREQUEST
 *
 * Identical on the wire in both states: `ciaddr` filled in, no
 * `requested IP address` or `server identifier` option, per
 * [RFC 2131, section 4.3.2](https://tools.ietf.org/html/rfc2131#section-4.3.2).
 * The two states only differ in where the client sends it.
 */
static size_t _build_request_bound(void)
{
    dhcpv4_msg_t *msg = (dhcpv4_msg_t *)send_buf;
    uint8_t msg_type = DHCPV4_MSG_REQUEST;
    size_t len = sizeof(dhcpv4_msg_t);

    _build_msg_hdr(msg, &lease.addr);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_MSG_TYPE, &msg_type, 1);
    len += _add_param_req_list(&send_buf[len]);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_END, NULL, 0);
    return len;
}

static uint32_t _get_opt_u32(const uint8_t *val)
{
    network_uint32_t tmp;

    memcpy(&tmp, val, sizeof(tmp));
    return byteorder_ntohl(tmp);
}

static void _parse_classless_routes(const uint8_t *val, uint8_t val_len,
                                    dhcpv4_client_lease_t *lease_out)
{
    size_t i = 0;

    while ((i < val_len) &&
           (lease_out->routes_numof < CONFIG_DHCPV4_CLIENT_ROUTE_MAX)) {
        uint8_t dst_len = val[i++];
        uint8_t dst_bytes = (dst_len + 7) / 8;
        dhcpv4_client_route_t *route;

        if (dst_len > 32) {
            DEBUG("dhcpv4_client: invalid classless route prefix length\n");
            break;
        }
        if ((i + dst_bytes + 4) > val_len) {
            DEBUG("dhcpv4_client: classless route option truncated\n");
            break;
        }
        route = &lease_out->routes[lease_out->routes_numof];
        memset(route, 0, sizeof(*route));
        memcpy(route->dst.u8, &val[i], dst_bytes);
        i += dst_bytes;
        memcpy(&route->gateway, &val[i], sizeof(route->gateway));
        i += sizeof(route->gateway);
        route->dst_len = dst_len;
        lease_out->routes_numof++;
    }
}

/**
 * @brief   Parses and validates a received message
 *
 * @return  true, if @p buf is a well-formed reply to the request currently
 *          outstanding (i.e. `op == BOOTREPLY`, magic cookie and `xid`
 *          match, and a message type option is present).
 */
static bool _parse_msg(const uint8_t *buf, size_t len, _parsed_msg_t *out)
{
    const dhcpv4_msg_t *msg = (const dhcpv4_msg_t *)buf;
    const uint8_t *opts;
    size_t opts_len;
    const uint8_t *val;
    uint8_t val_len;

    if (len < DHCPV4_MSG_MIN_SIZE) {
        DEBUG("dhcpv4_client: message too short\n");
        return false;
    }
    if (byteorder_ntohl(msg->magic_cookie) != DHCPV4_MAGIC_COOKIE) {
        DEBUG("dhcpv4_client: bad magic cookie\n");
        return false;
    }
    if (msg->op != DHCPV4_OP_BOOTREPLY) {
        return false;
    }
    if (byteorder_ntohl(msg->xid) != xid) {
        DEBUG("dhcpv4_client: transaction ID mismatch\n");
        return false;
    }

    opts = &buf[DHCPV4_MSG_MIN_SIZE];
    opts_len = len - DHCPV4_MSG_MIN_SIZE;

    memset(out, 0, sizeof(*out));
    out->lease.addr = msg->yiaddr;
    out->lease.prefix_len = 32;
    out->lease.lease_time = UINT32_MAX;

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_MSG_TYPE, &val_len);
    if ((val == NULL) || (val_len < 1)) {
        DEBUG("dhcpv4_client: no message type option\n");
        return false;
    }
    out->type = val[0];

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_SERVER_ID, &val_len);
    if ((val != NULL) && (val_len >= sizeof(ipv4_addr_t))) {
        memcpy(&out->server_id, val, sizeof(out->server_id));
        out->has_server_id = true;
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_SUBNET_MASK, &val_len);
    if ((val != NULL) && (val_len >= sizeof(ipv4_addr_t))) {
        ipv4_addr_t mask;

        memcpy(&mask, val, sizeof(mask));
        out->lease.prefix_len = ipv4_addr_prefix_from_netmask(&mask);
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_ROUTER, &val_len);
    if ((val != NULL) && (val_len >= sizeof(ipv4_addr_t))) {
        memcpy(&out->lease.router, val, sizeof(out->lease.router));
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_LEASE_TIME, &val_len);
    if ((val != NULL) && (val_len >= 4)) {
        out->lease.lease_time = _get_opt_u32(val);
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_RENEWAL_TIME, &val_len);
    if ((val != NULL) && (val_len >= 4)) {
        out->t1 = _get_opt_u32(val);
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_REBINDING_TIME, &val_len);
    if ((val != NULL) && (val_len >= 4)) {
        out->t2 = _get_opt_u32(val);
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_DNS_SERVER, &val_len);
    if ((val != NULL) && (val_len >= sizeof(ipv4_addr_t))) {
        memcpy(&out->dns, val, sizeof(out->dns));
        out->has_dns = true;
    }

    val = dhcpv4_opt_get(opts, opts_len, DHCPV4_OPT_CLASSLESS_ROUTES, &val_len);
    if (val != NULL) {
        _parse_classless_routes(val, val_len, &out->lease);
    }

    return true;
}

static void _release(bool notify_stack)
{
    _clear_event_timeout(&t1_timeout);
    _clear_event_timeout(&t2_timeout);
    if (notify_stack && (lease.addr.u32.u32 != 0)) {
        dhcpv4_client_release_lease(netif);
    }
    memset(&lease, 0, sizeof(lease));
    state = STATE_INIT;
}

static void _apply_ack(const _parsed_msg_t *parsed)
{
    lease = parsed->lease;
    if (parsed->has_server_id) {
        server_id = parsed->server_id;
    }
    lease_start_sec = _now_sec();

    if (lease.lease_time == UINT32_MAX) {
        /* infinite lease: never renew or rebind */
        t1_sec = 0;
        t2_sec = 0;
    }
    else {
        t1_sec = (parsed->t1 != 0) ? parsed->t1 : (lease.lease_time / 2);
        t2_sec = (parsed->t2 != 0) ? parsed->t2 : ((lease.lease_time * 7) / 8);
    }

    state = STATE_BOUND;
    _clear_event_timeout(&t1_timeout);
    _clear_event_timeout(&t2_timeout);
    if (t1_sec > 0) {
        _set_event_timeout_sec(&t1_timeout, &renew_event, t1_sec);
    }
    if (t2_sec > 0) {
        _set_event_timeout_sec(&t2_timeout, &rebind_event, t2_sec);
    }

    dhcpv4_client_conf_lease(netif, &lease);
#if IS_USED(MODULE_DHCPV4_CLIENT_DNS)
    if (parsed->has_dns) {
        dhcpv4_client_dns_conf(&parsed->dns, netif);
    }
#endif

    DEBUG("dhcpv4_client: bound, renew in %" PRIu32 "s, rebind in %" PRIu32
          "s\n", t1_sec, t2_sec);
}

static void _init(event_t *event)
{
    uint32_t rt_ms = CONFIG_DHCPV4_CLIENT_INITIAL_RT_MS;
    size_t len;

    (void)event;
    _release(false);
    state = STATE_SELECTING;
    _generate_xid();
    transaction_start_sec = _now_sec();
    len = _build_discover();

    DEBUG("dhcpv4_client: send DISCOVER\n");
    _flush_stale_replies();
    while (sock_udp_send(&sock, send_buf, len, &remote_broadcast) <= 0) {}

    while (true) {
        _parsed_msg_t parsed;
        int res = sock_udp_recv(&sock, recv_buf, sizeof(recv_buf),
                                rt_ms * US_PER_MS, NULL);

        if ((res > 0) && _parse_msg(recv_buf, res, &parsed) &&
            (parsed.type == DHCPV4_MSG_OFFER) && parsed.has_server_id) {
            DEBUG("dhcpv4_client: received OFFER\n");
            offered_addr = parsed.lease.addr;
            server_id = parsed.server_id;
            event_post(event_queue, &request_event);
            return;
        }
        rt_ms = _next_rt_ms(rt_ms);
        _set_secs((dhcpv4_msg_t *)send_buf);
        DEBUG("dhcpv4_client: resend DISCOVER\n");
        sock_udp_send(&sock, send_buf, len, &remote_broadcast);
    }
}

static void _request(event_t *event)
{
    uint32_t rt_ms = CONFIG_DHCPV4_CLIENT_INITIAL_RT_MS;
    unsigned retries = 0;
    size_t len;

    (void)event;
    state = STATE_REQUESTING;
    len = _build_request_selecting();

    DEBUG("dhcpv4_client: send REQUEST\n");
    _flush_stale_replies();
    while (sock_udp_send(&sock, send_buf, len, &remote_broadcast) <= 0) {}

    while (true) {
        _parsed_msg_t parsed;
        int res = sock_udp_recv(&sock, recv_buf, sizeof(recv_buf),
                                rt_ms * US_PER_MS, NULL);

        if ((res > 0) && _parse_msg(recv_buf, res, &parsed)) {
            if (parsed.type == DHCPV4_MSG_ACK) {
                DEBUG("dhcpv4_client: received ACK\n");
                _apply_ack(&parsed);
                return;
            }
            else if (parsed.type == DHCPV4_MSG_NAK) {
                DEBUG("dhcpv4_client: received NAK, restarting\n");
                _schedule_restart(random_uint32_range(
                                      0, CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS));
                return;
            }
        }
        if (++retries >= CONFIG_DHCPV4_CLIENT_REQUEST_MAX_RC) {
            DEBUG("dhcpv4_client: no reply to REQUEST, restarting\n");
            _schedule_restart(random_uint32_range(
                                  0, CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS));
            return;
        }
        rt_ms = _next_rt_ms(rt_ms);
        _set_secs((dhcpv4_msg_t *)send_buf);
        DEBUG("dhcpv4_client: resend REQUEST\n");
        sock_udp_send(&sock, send_buf, len, &remote_broadcast);
    }
}

/**
 * @brief   Shared RENEWING/REBINDING implementation
 *
 * @param[in] deadline_sec  Absolute (`_now_sec()`-relative) deadline of this
 *                          state: T2 for RENEWING, lease expiry for
 *                          REBINDING. Bounds the retry loop so it hands off
 *                          to the next state's already-armed event timeout
 *                          (or, for REBINDING, to expiry) rather than
 *                          retrying forever.
 * @param[in] dst           Where to send the DHCPREQUEST: the known server
 *                          for RENEWING, the broadcast address for
 *                          REBINDING.
 */
static bool _renew_rebind(uint32_t deadline_sec, const sock_udp_ep_t *dst)
{
    uint32_t rt_ms = CONFIG_DHCPV4_CLIENT_INITIAL_RT_MS;
    size_t len = _build_request_bound();

    DEBUG("dhcpv4_client: send REQUEST (renew/rebind)\n");
    _flush_stale_replies();
    while (sock_udp_send(&sock, send_buf, len, dst) <= 0) {}

    while (true) {
        uint32_t now = _now_sec();
        uint32_t mrd_ms;
        uint32_t timeout_ms;
        _parsed_msg_t parsed;
        int res;

        if (now >= deadline_sec) {
            return false;
        }
        mrd_ms = (deadline_sec - now) * MS_PER_SEC;
        timeout_ms = (rt_ms < mrd_ms) ? rt_ms : mrd_ms;
        res = sock_udp_recv(&sock, recv_buf, sizeof(recv_buf),
                            timeout_ms * US_PER_MS, NULL);
        if ((res > 0) && _parse_msg(recv_buf, res, &parsed)) {
            if (parsed.type == DHCPV4_MSG_ACK) {
                DEBUG("dhcpv4_client: received ACK (renew/rebind)\n");
                _apply_ack(&parsed);
                return true;
            }
            else if (parsed.type == DHCPV4_MSG_NAK) {
                DEBUG("dhcpv4_client: received NAK (renew/rebind)\n");
                _release(true);
                return true;
            }
        }
        rt_ms = _next_rt_ms(rt_ms);
        _set_secs((dhcpv4_msg_t *)send_buf);
        DEBUG("dhcpv4_client: resend REQUEST (renew/rebind)\n");
        sock_udp_send(&sock, send_buf, len, dst);
    }
}

static void _renew(event_t *event)
{
    (void)event;
    state = STATE_RENEWING;
    transaction_start_sec = _now_sec();
    remote_server.addr.ipv4_u32 = server_id.u32.u32;
    remote_server.netif = netif;

    if (_renew_rebind(lease_start_sec + t2_sec, &remote_server)) {
        return;
    }
    /* mrd elapsed without a definitive reply: t2_timeout is already about
     * to (or just did) fire rebind_event, nothing more to do here */
}

static void _rebind(event_t *event)
{
    (void)event;
    state = STATE_REBINDING;
    transaction_start_sec = _now_sec();

    if (_renew_rebind(lease_start_sec + lease.lease_time, &remote_broadcast)) {
        return;
    }
    DEBUG("dhcpv4_client: lease expired, restarting\n");
    _release(true);
    _schedule_restart(random_uint32_range(0, CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS));
}

static void _decline(event_t *event)
{
    dhcpv4_msg_t *msg = (dhcpv4_msg_t *)send_buf;
    uint8_t msg_type = DHCPV4_MSG_DECLINE;
    ipv4_addr_t addr;
    size_t len = sizeof(dhcpv4_msg_t);

    (void)event;
    if ((state == STATE_INIT) || (state == STATE_SELECTING)) {
        DEBUG("dhcpv4_client: nothing to decline\n");
        return;
    }
    addr = (state == STATE_REQUESTING) ? offered_addr : lease.addr;

    _generate_xid();
    transaction_start_sec = _now_sec();
    _build_msg_hdr(msg, NULL);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_MSG_TYPE, &msg_type, 1);
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_REQUESTED_IP, &addr,
                          sizeof(addr));
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_SERVER_ID, &server_id,
                          sizeof(server_id));
    len += dhcpv4_opt_add(&send_buf[len], DHCPV4_OPT_END, NULL, 0);

    DEBUG("dhcpv4_client: send DECLINE\n");
    sock_udp_send(&sock, send_buf, len, &remote_broadcast);

    _release(true);
    _schedule_restart(CONFIG_DHCPV4_CLIENT_DECLINE_DELAY_MS);
}

void dhcpv4_client_decline(void)
{
    event_post(event_queue, &decline_event);
}

void dhcpv4_client_init(event_queue_t *eq, uint16_t netif_pid)
{
    assert(eq->waiter != NULL);
    event_queue = eq;
    netif = netif_pid;
    local.netif = netif_pid;
    remote_broadcast.netif = netif_pid;
}

void dhcpv4_client_start(void)
{
    int res = dhcpv4_client_get_hwaddr(&netif, hwaddr, &hwaddr_len);

    assert(event_queue != NULL);
    if (res < 0) {
        LOG_ERROR("dhcpv4_client: unable to get hardware address of "
                  "interface %u\n", netif);
        return;
    }
    htype = (uint8_t)res;
    /* dhcpv4_client_get_hwaddr() may have resolved SOCK_ADDR_ANY_NETIF to a
     * concrete interface; every later send/receive must target that same
     * interface, not the wildcard */
    local.netif = netif;
    remote_broadcast.netif = netif;
    sock_udp_create(&sock, &local, NULL, 0);
    _schedule_restart(random_uint32_range(0, CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS));
}

#ifdef MODULE_AUTO_INIT_DHCPV4_CLIENT
static char _thread_stack[DHCPV4_CLIENT_STACK_SIZE];
static void *_thread(void *args);
static kernel_pid_t _thread_pid;

void dhcpv4_client_auto_init(void)
{
    if (_thread_pid <= 0) {
        _thread_pid = thread_create(_thread_stack, sizeof(_thread_stack),
                                    DHCPV4_CLIENT_PRIORITY, 0,
                                    _thread, NULL, "dhcpv4-client");
    }
}

static void *_thread(void *args)
{
    (void)args;
    event_queue_t auto_init_event_queue;

    event_queue_init(&auto_init_event_queue);
    dhcpv4_client_init(&auto_init_event_queue, SOCK_ADDR_ANY_NETIF);
    dhcpv4_client_start();
    event_loop(&auto_init_event_queue); /* never returns */
    return NULL;
}
#endif /* MODULE_AUTO_INIT_DHCPV4_CLIENT */

/** @} */
