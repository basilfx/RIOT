# gnrc_knx_taster
Source code for the BasilFX Taster.

## Preparation
You need to provide `property_types.h` based on a `knx_master.xml` (typically
installed with ETS5). Due to licensing, this file is not included. A Python
script is included that can generate this file.

First install the required dependencies, using Pip:

```shell
pip install -r dist/requirements.txt
```

The, to generate the required file, run:

```shell
python3 dist/generate_property_types.py path/to/knx_master.xml > property_types.h
```

## Configuration
Change `Makefile` to include the KNX transceiver of choice (defaults to
`ncn5120`).

## File system
This device makes use of the file system to store device configuration. The
following files exist:

* `/mtd0/magic`
* `/mtd0/version`
* `/mtd0/address`
* `/mtd0/serial`

The files will be created when programmed by ETS5. The file system can be
formatted using the `reset` command.

## Commands
If the shell is enabled, several commands are available to interact with the
device:

* `reset` — reset the device (formats the flash memory)
* `settings` — print overview of settings
