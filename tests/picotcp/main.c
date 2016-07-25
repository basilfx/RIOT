#include "pico_stack.h"

#define PICOTCP_STACKSIZE  (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_PRIO       (THREAD_PRIORITY_MAIN - 3)
#define PICOTCP_DELAY      (500)

static char picotcp_stack[PICOTCP_STACKSIZE];

static void *_picotcp_thread(void *args)
{
    /* initialize the stack */
    pico_stack_init();

    /* process data and events */
    while (1) {
        pico_stack_tick();
    }

    /* should be never reached */
    return NULL;
}

int main(void)
{
    thread_create(picotcp_stack, sizeof(picotcp_stack), PICOTCP_PRIO,
                  THREAD_CREATE_STACKTEST, _picotcp_thread, NULL, "picotcp");

    /* should be never reached */
    return 0;
}
