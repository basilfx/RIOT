# NCN5120 Driver Test

## Introduction
This test checks if the ON Semiconductor NCN5120 transceiver is working.

## Expected result
When started, this test will run some of the core functions, like setting an
address, setting configuration parameters and sending a telegram to the device.

It should finish with the message 'Tests succeeded'.

This test does not verify the netdev interface.

## SAVE pin
By default, the SAVE pin is not used. If your transceiver has the SAVE pin
connected, you can test this features by removing the bus voltage. A message
should be printed on screen.
