# About
This is a test application for the ST TDA74xx audio processors.

# Usage
This test application will initialize all audio processors and provide a shell
for interacting with them.

By default, this test expects the TDA7440. Multiple variations of the TDA74xx
can be tested at once, by enabling multiple drivers using `USEMODULE`. See the
`Makefile` for the possible options.

The volume is muted after initialization, therefore `set_volume` (or
`set_mute 0`) must be used before any sound is produced.

The address of all devices is fixed, therefore testing multiple devices at once
requires them to be on separate I2C buses. The bus and the address of a device
can be overridden per variation, by overriding `<variation>_PARAM_I2C_DEV` and
`<variation>_PARAM_ADDR` (e.g. `TDA7440_PARAM_I2C_DEV`).
