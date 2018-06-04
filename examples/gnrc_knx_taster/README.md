# gnrc_knx_taster
Source code for the BasilFX Taster.

## Preparation
You need to provide `property_types.h` based on a `knx_master.xml`. Due to licensing, this file is not included.

A Python script is included that can generate this file. Invoke it with `python dist/generate_property_types.py path/to/knx_master.xml > property_types.h`. You need to have Python 3.6 installed, together with BeautifulSoup 4 and LXML parser.

## Configuration
Change `Makefile` to include the KNX transceiver of choice (defaults to `ncn5120`).

## File system
This device makes use of the file system to store device configuration. The following files exist:

* `/mtd0/magic`
* `/mtd0/version`
* `/mtd0/address`
* `/mtd0/serial`

The files will be created when programmed by ETS5. The file system can be formatted using the `reset` command.

## Commands
If the shell is enabled, several commands are available to interact with the device:

* `reset` — reset the device (formats the flash memory)
* `settings` — print overview of settings
