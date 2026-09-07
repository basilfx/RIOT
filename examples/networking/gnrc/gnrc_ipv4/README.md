# gnrc_ipv4 example

This example shows GNRC's IPv4 support: ARP, ICMPv4 echo (`ping4`) and
errors, IGMPv2 group membership, UDP and fragmentation. It has no IPv6
module, so it demonstrates an IPv4-only build. See the
[IPv4 networking guide][guide] for how an IPv4-only build compares to an
IPv6-only or dual-stack one.

## Connecting RIOT native and the Linux host

First, make sure you have compiled the application by calling `make`.

Now, create a tap interface:

    sudo ./../../../../dist/tools/tapsetup/tapsetup

This should automatically connect to the `tap0` interface. If this does not
work for any reason, run `make term` with the tap interface as the `PORT`
environment variable:

    PORT=tap0 make term

The node starts without an IPv4 address. Configure one from the shell:

    > ifconfig 6 add 192.168.0.100/24
    success: added 192.168.0.100/24 to interface 6

(Replace `6` with the interface number reported by `ifconfig` on your setup.)
Then, on the Linux host, give the tap interface an address on the same
subnet and bring it up:

    sudo ip addr add 192.168.0.1/24 dev tap0
    sudo ip link set tap0 up

To verify connectivity, ping the RIOT node **from the Linux host**:

    ping 192.168.0.100

and ping the Linux host **from the RIOT shell**:

    > ping4 192.168.0.1
    12 bytes from 192.168.0.1: icmp_seq=0 ttl=64 time=0.379 ms
    ...

If the pings succeed, start a UDP server on the RIOT node:

    > udp server start 8808
    Success: started UDP server on port 8808

and send it a message from the Linux host with netcat:

    nc -4u 192.168.0.100 8808

You should see the message appear in the RIOT shell.

## Connecting two RIOT instances

Set up two tap devices and a bridge that connects them:

    sudo ./../../../../dist/tools/tapsetup/tapsetup --create 2

Build and start the first instance with `make term`, then configure and
note its address:

    > ifconfig 6 add 192.168.0.1/24

Start a second instance, this time on `tap1`:

    PORT=tap1 make term

and configure it on the same subnet:

    > ifconfig 6 add 192.168.0.2/24

From the second instance, ping the first:

    > ping4 192.168.0.1

## Configuring the address automatically

Rather than typing `ifconfig` by hand every time, the `Makefile` has two
commented-out options:

* Uncomment the `gnrc_ipv4_static_addr` block and adjust the address,
  prefix length and default router to have them configured automatically
  at startup.
* Uncomment `gnrc_dhcpv4_client` to obtain an address, default router and
  subnet mask from a DHCPv4 server on the network instead. This is useful
  when running against a real Ethernet uplink rather than a tap interface.

## Multicast

The node automatically joins the all-hosts group `224.0.0.1` and reports it
via IGMPv2, so a query on the subnet (for example from the Linux host with
`ip maddr` or by running an IGMP querier) shows the node as a member.
Additional groups can be joined and left with `ifconfig`:

    > ifconfig 6 add 224.0.0.100
    > ifconfig 6 del 224.0.0.100

[guide]: https://guide.riot-os.org/networking/ipv4
