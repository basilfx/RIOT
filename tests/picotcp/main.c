
#include "thread.h"
#include "shell.h"

#include "pico_stack.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"
#include "xtimer.h"

#define PICOTCP_STACKSIZE  (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_PRIO       (THREAD_PRIORITY_MAIN - 3)
#define PICOTCP_DELAY      (500)

#define NUM_PING 1

static char picotcp_stack[PICOTCP_STACKSIZE];

static void *_picotcp_thread(void *args)
{
    (void) args;
    /* process data and events */
    while (1) {
        pico_stack_tick();
        xtimer_usleep(1000);
    }

    /* should be never reached */
    return NULL;
}

/* gets called when the ping receives a reply, or encounters a problem */
void _cb_ping(struct pico_icmp4_stats *s)
{
    char host[30];
    pico_ipv4_to_string(host, s->dst.addr);
    if (s->err == 0) {
        /* if all is well, print some pretty info */
        printf("%lu bytes from %s: icmp_req=%lu ttl=%lu time=%lu ms\n", s->size,
                host, s->seq, s->ttl, (long unsigned int)s->time);
    } else {
        /* if something went wrong, print it and signal we want to stop */
        printf("PING %lu to %s: Error %d\n", s->seq, host, s->err);
    }
}
static int _ping(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int id;
    struct pico_ip4 ipaddr, netmask;

    /* assign the IP address to the tap interface */
    pico_string_to_ipv4("192.168.5.4", &ipaddr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    pico_ipv4_link_add(pico_dev, ipaddr, netmask);

    printf("starting ping\n");
    id = pico_icmp4_ping("192.168.5.5", NUM_PING, 1000, 10000, 64, _cb_ping);

    if (id == -1)
        return -1;
    else
        return 0;
}

static const shell_command_t shell_commands[] = {
    { "ping", "Send pings", _ping },
    { NULL, NULL, NULL }
};
int main(void)
{
    /* initialize the stack */
    pico_stack_init();

    thread_create(picotcp_stack, sizeof(picotcp_stack), PICOTCP_PRIO,
                  THREAD_CREATE_STACKTEST, _picotcp_thread, NULL, "picotcp");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
