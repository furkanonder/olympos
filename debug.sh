#!/bin/sh
set -e
. ./iso.sh

# Start QEMU with debugging capabilities in the background
# -s: Shorthand for -gdb tcp::1234 (starts GDB server on port 1234)
# -S: Pause CPU at startup (wait for debugger to connect and continue)
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom olympos.iso -s -S & QEMU_PID=$!

# Give QEMU a moment to start
sleep 1

echo "Starting GDB and connecting to QEMU..."
echo "If you quit GDB, the QEMU process will be terminated automatically."

# Connect GDB to QEMU
gdb -ex "target remote localhost:1234" \
             -ex "symbol-file isodir/boot/olympos.kernel" \
             -ex "break kernel_main" \
             -ex "continue"

# Clean up the QEMU process when GDB exits
kill $QEMU_PID 2>/dev/null || true
