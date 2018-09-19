# gnrc_knx_device
Minimal example of a KNX device.

# Introduction
Two communications are exposed and will react to bus updates. LED0 responds to communication object
writes, while PB0 will generate communication object writes.

## Details
This example uses a static address of 3.2.1 and provides two communication objects:

- Communication object 0 is mapped to LED0 with group address 0/0/1
- Communication object 1 is mapped to PB0 with group address 0/0/2

In this example, memory segments and properties are not used, and therefore this device cannot be
configured using the ETS5 (KNX engineering tool).

You can use ETS5 (and many other KNX tools) to monitor bus traffic and interact with this device.

## Interaction
A command prompt is available. Use the `knx` command to interact with the device definition.

## Configuration
Change `Makefile` to include the KNX transceiver of choice (defaults to `ncn5120`). Refer to
`tests/driver_ncn5120` or `tests/driver_tpuart` for more information about these drivers.

Both device drivers require UART 8E1, which is not supported by all MCUs.

## Hardware
The easiest way to get started, is to acquire a SIEMENS 5WG1117-2AB12. This bus coupler unit
contains a TPUART transceiver. The pinout of this unit is:

* Pin 1: GND
* Pin 2: RX
* Pin 4: TX
* Pin 5: VCC

Note that pin 1 is the lower left pin and pin 10 the top left pin, when the KNX contacts face
upwards.

## Next steps
This is a minimal example, and uses pre-configured addresses and associations. A typical device can
be programmed (configured) remotely. Please take a look at `gnrc_knx_l7` module for more
information.
