

#define ENABLE_DEBUG    (0)
#include "debug.h"

static gnrc_nrfnet_netif_t nrfnet_ifs[GNRC_NETIF_NUMOF];



void gnrc_nrfnet_netif_init(void)
{
    for (int i = 0; i < GNRC_NETIF_NUMOF; i++) {
        mutex_init(&(nrfnet_ifs[i].mutex));
    }
}

void gnrc_nrfnet_netif_add(kernel_pid_t pid)
{
    gnrc_nrfnet_netif_t *free_entry = NULL;

    for (int i = 0; i < GNRC_NETIF_NUMOF; i++) {
        if (nrfnet_ifs[i].pid == pid) {
            /* pid has already been added */
            return;
        }

        else if ((nrfnet_ifs[i].pid == KERNEL_PID_UNDEF) && !free_entry) {
            /* found the first free entry */
            free_entry = &nrfnet_ifs[i];
        }
    }

    if (!free_entry) {
        DEBUG("gnrc_nrfnet_netif: could not add %" PRIkernel_pid " to nRFNet: No space left.\n", pid);
        return;
    }

    /* otherwise, fill the free entry */
    mutex_lock(&free_entry->mutex);

    DEBUG("gnrc_nrfnet_netif: add nRFNet interface %" PRIkernel_pid " (i = %d)\n", pid, free_entry - nrfnet_ifs);

    free_entry->pid = pid;
    free_entry->max_frag_size = 255;

    _add_addr_to_entry(free_entry, &nrfnet_addr_all_nodes_link_local,
                       IPV6_ADDR_BIT_LEN, 0);

    mutex_unlock(&free_entry->mutex);
}

nrfnet_addr_t *gnrc_nrfnet_netif_add_addr(kernel_pid_t pid, const nrfnet_addr_t *addr)
{
    gnrc_nrfnet_netif_t *entry = gnrc_nrfnet_netif_get(pid);
    nrfnet_addr_t *res;

    if ((entry == NULL) || (addr == NULL) || (nrfnet_addr_is_unspecified(addr)) ||
        ((prefix_len - 1) > 127)) {    /* prefix_len < 1 || prefix_len > 128 */
        return NULL;
    }

    mutex_lock(&entry->mutex);

    res = _add_addr_to_entry(entry, addr, prefix_len, flags);

    mutex_unlock(&entry->mutex);

    return res;
}

void gnrc_nrfnet_netif_remove(kernel_pid_t pid)
{

}


gnrc_nrfnet_netif_t *gnrc_nrfnet_netif_get(kernel_pid_t pid)
{
    for (int i = 0; i < GNRC_NETIF_NUMOF; i++) {
        if (nrfnet_ifs[i].pid == pid) {
            DEBUG("gnrc_nrfnet_netif: get nRFNet interface %" PRIkernel_pid " (%p, i = %d)\n", pid, (void *)(&(nrfnet_ifs[i])), i);
            return &(nrfnet_ifs[i]);
        }
    }

    return NULL;
}
