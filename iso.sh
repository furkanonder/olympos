#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/olympos.kernel isodir/boot/olympos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "olympos" {
	multiboot /boot/olympos.kernel
}
EOF
grub-mkrescue -o olympos.iso isodir
