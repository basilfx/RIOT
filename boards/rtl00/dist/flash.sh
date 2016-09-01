#!/bin/sh
#
# Firmware header creator.
#
# This script is supposed to be called from RIOTs
# unified OpenOCD script (dist/tools/openocd/openocd.sh).
#
# @author       Bas Stottelaar <basstottelaar@gmail.com>

JUMP_ADDRESS=`arm-none-eabi-readelf -S $ELFFILE | grep .text | awk '{print $5}'`

# Create binary from ELF.
arm-none-eabi-objcopy -O binary $ELFFILE $ELFFILE.bin

# Compile header.
#arm-none-eabi-gcc -mthumb -mcpu=cortex-m3 -DADDRESS="0x$ADDRESS" -DFILENAME="\"$ELFFILE.bin\"" -nostartfiles -nostdlib -Wl,-Ttext="0" "$RIOTBASE/boards/rtl00/dist/header.S" -o flash.elf

# Convert to binary from ELF.
#arm-none-eabi-objcopy -O binary flash.elf flash.bin

# Now flash it.
echo "JUMP TO $JUMP_ADDRESS"
openocd -f "$RIOTBASE/boards/rtl00/dist/buspirate.cfg" -f "$RIOTBASE/boards/rtl00/dist/rtl8710.ocd" -c "init" -c "reset halt" -c "load_image $ELFFILE 0x0 elf" -c "cortex_bootstrap 0x$JUMP_ADDRESS" -c "halt"
