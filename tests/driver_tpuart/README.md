# TPUART Driver Test

## Introduction
This test checks if the Siemens TPUART transceiver is working.

**NOTE:** this test requires a UART device that support 8E1 (even parity). This is not exposed by the UART interface
provided by RIOT-OS, so you have to configure it separately.

## Expected result
When started, this test will run some of the core functions, like setting an address, enabling checksums and sending a
telegram to the device. It should finish with the message 'Tests succeeded'.

This test does not verify the netdev interface.

### SAVE pin
By default, the SAVE pin is not used. If your transceiver has the SAVE pin connected, you can test this features by
removing the bus voltage. A message should be printed on screen.
