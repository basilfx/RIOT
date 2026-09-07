GNRC dual-stack build test
===========================

This tests that a GNRC application built with both `gnrc_ipv4` and
`gnrc_ipv6_default` (plus the family-aware upper layers on top of them:
`gnrc_udp`, `gnrc_tcp`, `gnrc_sock_udp`, `gnrc_sock_ip`, `gnrc_sock_tcp`)
compiles, links and boots both network-layer threads.

It intentionally has no netdev/netif module: it is a compile and boot smoke
test, not a networking test, and it should not need a tap interface (or
root) to run. Nothing else in the test suite builds this module
combination, so it is the only thing exercising the dual-stack
conditional-compilation surface in CI.
