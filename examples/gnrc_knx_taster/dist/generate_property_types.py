#!/usr/bin/env python3

# Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

from bs4 import BeautifulSoup
from collections import defaultdict

import sys


def main(argv):
    """
    Application entry point.
    """

    if len(argv) == 1:
        sys.stderr.write("error: missing path to knx_master.xml\n")
        return 1

    with open(argv[1], "r") as fp:
        # Hack: skip the BOM that is present.
        fp.read(1)
        soup = BeautifulSoup(fp, "lxml-xml")

        # Generate file header.
        sys.stdout.write(
            "/* This is an auto-generated file. " +
            "Modifications will be overwritten. */\n")
        sys.stdout.write("\n")

        sys.stdout.write("#ifndef PROPERTY_TYPES_H\n")
        sys.stdout.write("#define PROPERTY_TYPES_H\n")
        sys.stdout.write("\n")

        sys.stdout.write("#ifdef __cplusplus\n")
        sys.stdout.write("extern \"C\" {\n")
        sys.stdout.write("#endif\n")
        sys.stdout.write("\n")

        types = soup \
            .find("InterfaceObjectTypes") \
            .find_all("InterfaceObjectType")
        properties = soup \
            .find("InterfaceObjectProperties") \
            .find_all("InterfaceObjectProperty")

        # Generate types as C-enum.
        sys.stdout.write("/**\n")
        sys.stdout.write(" * @brief Object types\n")
        sys.stdout.write(" */\n")
        sys.stdout.write("enum {\n")

        for type_ in types:
            try:
                sys.stdout.write(f"    /**< {type_['Text']} */\n")
            except KeyError:
                pass
            sys.stdout.write(f"    {type_['Name']} = {type_['Number']},\n")

        sys.stdout.write("};\n")
        sys.stdout.write("\n")

        # Generate properties as a C-enum.
        sys.stdout.write("/**\n")
        sys.stdout.write(" * @brief Property types\n")
        sys.stdout.write(" */\n")
        sys.stdout.write("enum {\n")

        history = defaultdict(int)

        for property_ in properties:
            try:
                name = property_['ObjectType'].replace("OT-", "")
                name = property_['Name'].replace("PID_", "PID_" + name + "_")
            except KeyError:
                name = property_['Name'].replace("PID_", "PID_G_")

            name = name.replace("%", "PERCENTAGE")
            append = f"_{history[name]}" if history[name] > 0 else ""

            try:
                sys.stdout.write(f"    /**< {property_['Text']} */\n")
            except KeyError:
                pass

            sys.stdout.write(f"    {name}{append} = {property_['Number']},\n")
            history[name] += 1

        # Generate file footer.
        sys.stdout.write("};\n")
        sys.stdout.write("\n")

        sys.stdout.write("#ifdef __cplusplus\n")
        sys.stdout.write("}\n")
        sys.stdout.write("#endif\n")
        sys.stdout.write("\n")

        sys.stdout.write("#endif /* PROPERTY_TYPES_H */\n")
        sys.stdout.write("\n")


# E.g. `python3 generate_property_types.py knx_master.xml > property_types.h`.
if __name__ == "__main__":
    sys.exit(main(sys.argv))
