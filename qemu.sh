#!/bin/sh
set -e
. ./iso.sh

# -cdrom olympos.iso: Use the Olympos ISO image as a CD-ROM
# -serial file:serial.log: Redirect serial port output to a log file for debugging
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom olympos.iso -serial file:serial.log
