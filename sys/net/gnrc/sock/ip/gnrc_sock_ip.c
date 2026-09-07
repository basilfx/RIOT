/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       GNRC implementation of @ref net_sock_ip
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "byteorder.h"
#include "net/af.h"
#include "net/protnum.h"
#include "net/gnrc/ipv6.h"
#include "net/sock/ip.h"
#include "random.h"

#include "gnrc_sock_internal.h"

#if IS_USED(MODULE_GNRC_IPV4)
#include "net/ipv4/addr.h"
#endif

#ifdef SOCK_HAS_ASYNC_CTX
#include "net/sock/async/event.h"
#endif

int sock_ip_create(sock_ip_t *sock, const sock_ip_ep_t *local,
                   const sock_ip_ep_t *remote, uint8_t proto, uint16_t flags)
{
    int family;

    assert(sock);
    if ((local != NULL) && (remote != NULL) &&
        (local->netif != SOCK_ADDR_ANY_NETIF) &&
        (remote->netif != SOCK_ADDR_ANY_NETIF) &&
        (local->netif != remote->netif)) {
        return -EINVAL;
    }
    memset(&sock->local, 0, sizeof(sock_ip_ep_t));
    if (local != NULL) {
        if (gnrc_af_not_supported(local->family)) {
            return -EAFNOSUPPORT;
        }
        memcpy(&sock->local, local, sizeof(sock_ip_ep_t));
    }
    memset(&sock->remote, 0, sizeof(sock_ip_ep_t));
    if (remote != NULL) {
        if (gnrc_af_not_supported(remote->family)) {
            return -EAFNOSUPPORT;
        }
        if (gnrc_ep_addr_any(remote)) {
            return -EINVAL;
        }
        gnrc_ep_set(&sock->remote, remote, sizeof(sock_ip_ep_t));
    }

    /* net_gnrc_netreg has no notion of address family: pick the nettype to
     * register under from whichever endpoint was given. Neither given keeps
     * the pre-IPv4 default of preferring IPv6, when built in. */
    family = (local != NULL) ? local->family :
             ((remote != NULL) ? remote->family : AF_UNSPEC);
    switch (family) {
#if IS_USED(MODULE_GNRC_IPV4)
        case AF_INET:
            sock->type = GNRC_NETTYPE_IPV4;
            break;
#endif
#if IS_USED(MODULE_GNRC_IPV6)
        case AF_INET6:
            sock->type = GNRC_NETTYPE_IPV6;
            break;
#endif
        default:
#if IS_USED(MODULE_GNRC_IPV6)
            sock->type = GNRC_NETTYPE_IPV6;
#else
            sock->type = GNRC_NETTYPE_IPV4;
#endif
            break;
    }
    gnrc_sock_create(&sock->reg, sock->type, proto);
    sock->flags = flags;
    return 0;
}

void sock_ip_close(sock_ip_t *sock)
{
    assert(sock != NULL);
    /* GNRC_NETTYPE_UNDEF (the zero-initialized default, and what this sets
     * sock->type back to below) is never a real registration target for a
     * raw IP sock: skip unregistering with it, since nothing ever registers
     * under it and gnrc_netreg_unregister() would walk off the end of the
     * (always empty) list */
    if (sock->type != GNRC_NETTYPE_UNDEF) {
        gnrc_netreg_unregister(sock->type, &sock->reg.entry);
        sock->type = GNRC_NETTYPE_UNDEF;
    }
#ifdef SOCK_HAS_ASYNC_CTX
    sock_event_close(sock_ip_get_async_ctx(sock));
#endif
}

int sock_ip_get_local(sock_ip_t *sock, sock_ip_ep_t *local)
{
    assert(sock && local);
    if (sock->local.family == AF_UNSPEC) {
        return -EADDRNOTAVAIL;
    }
    memcpy(local, &sock->local, sizeof(sock_ip_ep_t));
    return 0;
}

int sock_ip_get_remote(sock_ip_t *sock, sock_ip_ep_t *remote)
{
    assert(sock && remote);
    if (sock->remote.family == AF_UNSPEC) {
        return -ENOTCONN;
    }
    memcpy(remote, &sock->remote, sizeof(sock_ip_ep_t));
    return 0;
}

ssize_t sock_ip_recv_aux(sock_ip_t *sock, void *data, size_t max_len,
                         uint32_t timeout, sock_ip_ep_t *remote,
                         sock_ip_aux_rx_t *aux)
{
    void *pkt = NULL, *ctx = NULL;
    uint8_t *ptr = data;
    ssize_t res, ret = 0;
    bool nobufs = false;

    assert((sock != NULL) && (data != NULL) && (max_len > 0));
    while ((res = sock_ip_recv_buf_aux(sock, &pkt, &ctx, timeout, remote, aux)) > 0) {
        if (res > (ssize_t)max_len) {
            nobufs = true;
            continue;
        }
        memcpy(ptr, pkt, res);
        ptr += res;
        ret += res;
    }
    return (nobufs) ? -ENOBUFS : ((res < 0) ? res : ret);
}

ssize_t sock_ip_recv_buf_aux(sock_ip_t *sock, void **data, void **buf_ctx,
                             uint32_t timeout, sock_ip_ep_t *remote,
                             sock_ip_aux_rx_t *aux)
{
    (void)aux;
    gnrc_pktsnip_t *pkt;
    sock_ip_ep_t tmp;
    int res;
    gnrc_sock_recv_aux_t _aux = { 0 };

    assert((sock != NULL) && (data != NULL) && (buf_ctx != NULL));
    if (*buf_ctx != NULL) {
        *data = NULL;
        gnrc_pktbuf_release(*buf_ctx);
        *buf_ctx = NULL;
        return 0;
    }
    if (sock->local.family == 0) {
        return -EADDRNOTAVAIL;
    }
    tmp.family = sock->local.family;
#if IS_USED(MODULE_SOCK_AUX_LOCAL)
    if ((aux != NULL) && (aux->flags & SOCK_AUX_GET_LOCAL)) {
        _aux.local = &aux->local;
    }
#endif
#if IS_USED(MODULE_SOCK_AUX_TIMESTAMP)
    if ((aux != NULL) && (aux->flags & SOCK_AUX_GET_TIMESTAMP)) {
        _aux.timestamp = &aux->timestamp;
    }
#endif
#if IS_USED(MODULE_SOCK_AUX_RSSI)
    if ((aux != NULL) && (aux->flags & SOCK_AUX_GET_RSSI)) {
        _aux.rssi = &aux->rssi;
    }
#endif
    res = gnrc_sock_recv((gnrc_sock_reg_t *)sock, &pkt, timeout, &tmp, &_aux);
    if (res < 0) {
        return res;
    }
    if (remote != NULL) {
        /* return remote to possibly block if wrong remote */
        memcpy(remote, &tmp, sizeof(tmp));
    }
    if (sock->remote.family != AF_UNSPEC) {   /* check remote end-point if set */
        size_t addr_size = sizeof(ipv6_addr_t);
        const void *unspec = &ipv6_addr_unspecified;

#if IS_USED(MODULE_GNRC_IPV4)
        if (sock->remote.family == AF_INET) {
            addr_size = sizeof(ipv4_addr_t);
            unspec = &ipv4_addr_unspecified;
        }
#endif
        if ((memcmp(&sock->remote.addr, unspec, addr_size) != 0) &&
            (memcmp(&sock->remote.addr, &tmp.addr, addr_size) != 0)) {
            gnrc_pktbuf_release(pkt);
            return -EPROTO;
        }
    }
#if IS_USED(MODULE_SOCK_AUX_LOCAL)
    if (aux != NULL) {
        aux->flags &= ~(SOCK_AUX_GET_LOCAL);
    }
#endif
#if IS_USED(MODULE_SOCK_AUX_TIMESTAMP)
    if ((aux != NULL) && (_aux.flags & GNRC_SOCK_RECV_AUX_FLAG_TIMESTAMP)) {
        aux->flags &= ~(SOCK_AUX_GET_TIMESTAMP);
    }
#endif
#if IS_USED(MODULE_SOCK_AUX_RSSI)
    if ((aux != NULL) && (_aux.flags & GNRC_SOCK_RECV_AUX_FLAG_RSSI)) {
        aux->flags &= ~(SOCK_AUX_GET_RSSI);
    }
#endif
    *data = pkt->data;
    *buf_ctx = pkt;
    res = (int)pkt->size;
    return res;
}

ssize_t sock_ip_send_aux(sock_ip_t *sock, const void *data, size_t len,
                         uint8_t proto, const sock_ip_ep_t *remote,
                         sock_ip_aux_tx_t *aux)
{
    (void)aux;
    int res;
    gnrc_pktsnip_t *pkt;
    sock_ip_ep_t local;
    sock_ip_ep_t rem;

    assert((sock != NULL) || (remote != NULL));
    assert((len == 0) || (data != NULL)); /* (len != 0) => (data != NULL) */
    if ((remote != NULL) && (sock != NULL) &&
        (sock->local.netif != SOCK_ADDR_ANY_NETIF) &&
        (remote->netif != SOCK_ADDR_ANY_NETIF) &&
        (sock->local.netif != remote->netif)) {
        return -EINVAL;
    }
    if ((remote == NULL) &&
        /* cppcheck-suppress nullPointerRedundantCheck
         * (reason: sock can't be NULL as per the check above) */
        (sock->remote.family == AF_UNSPEC)) {
        return -ENOTCONN;
    }
    else if ((remote != NULL) && (gnrc_ep_addr_any(remote))) {
        return -EINVAL;
    }
    /* cppcheck-suppress nullPointerRedundantCheck
     * (reason: compiler evaluates lazily so this isn't a redundundant check and
     * cppcheck is being weird here anyways) */
    if ((sock == NULL) || (sock->local.family == AF_UNSPEC)) {
        /* no sock or sock currently unbound */
        memset(&local, 0, sizeof(local));
    }
    else {
        if (sock != NULL) {
            proto = (uint8_t)sock->reg.entry.demux_ctx;
        }
        memcpy(&local, &sock->local, sizeof(local));
    }
#if IS_USED(MODULE_SOCK_AUX_LOCAL)
    /* user supplied local endpoint takes precedent */
    if ((aux != NULL) && (aux->flags & SOCK_AUX_SET_LOCAL)) {
        local = aux->local;
        aux->flags &= ~SOCK_AUX_SET_LOCAL;
    }
#endif
    if (remote == NULL) {
        /* sock can't be NULL at this point */
        memcpy(&rem, &sock->remote, sizeof(rem));
    }
    else {
        gnrc_ep_set(&rem, remote, sizeof(rem));
    }
    if ((remote != NULL) && (remote->family == AF_UNSPEC) &&
        (sock != NULL) && (sock->remote.family != AF_UNSPEC)) {
        /* remote was set on create so take its family */
        rem.family = sock->remote.family;
    }
    else if ((remote != NULL) && gnrc_af_not_supported(remote->family)) {
        return -EAFNOSUPPORT;
    }
    else if ((local.family == AF_UNSPEC) && (rem.family != AF_UNSPEC)) {
        /* local was set to 0 above */
        local.family = rem.family;
    }
    else if ((local.family != AF_UNSPEC) && (rem.family == AF_UNSPEC)) {
        /* local was given on create, but remote family wasn't given by user and
         * there was no remote given on create, take from local */
        rem.family = local.family;
    }
    pkt = gnrc_pktbuf_add(NULL, (void *)data, len, GNRC_NETTYPE_UNDEF);
    if (pkt == NULL) {
        return -ENOMEM;
    }
    res = gnrc_sock_send(pkt, &local, &rem, proto);
    if (res <= 0) {
        return res;
    }
#ifdef SOCK_HAS_ASYNC
    if ((sock != NULL) && (sock->reg.async_cb.ip)) {
        sock->reg.async_cb.ip(sock, SOCK_ASYNC_MSG_SENT,
                              sock->reg.async_cb_arg);
    }
#endif  /* SOCK_HAS_ASYNC */
    return res;
}

#ifdef SOCK_HAS_ASYNC
void sock_ip_set_cb(sock_ip_t *sock, sock_ip_cb_t cb, void *arg)
{
    sock->reg.async_cb_arg = arg;
    sock->reg.async_cb.ip = cb;
}

#ifdef SOCK_HAS_ASYNC_CTX
sock_async_ctx_t *sock_ip_get_async_ctx(sock_ip_t *sock)
{
    return &sock->reg.async_ctx;
}
#endif  /* SOCK_HAS_ASYNC_CTX */
#endif  /* SOCK_HAS_ASYNC */

/** @} */
