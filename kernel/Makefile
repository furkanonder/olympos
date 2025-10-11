# Get the default host from default-host.sh script
DEFAULT_HOST!=../default-host.sh
# Allow override of host, otherwise use default
HOST?=DEFAULT_HOST
# Extract architecture from host triplet (e.g., i386-elf -> i386)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

# Compiler flags
CFLAGS?=-O2 -g		# Default optimization level (-O2) and debugging info (-g)
CPPFLAGS?=		# C preprocessor flags
LDFLAGS?=		# Linker flags
LIBS?=			# Libraries to link against

# Installation directories
DESTDIR?=				# Used for staging area during installation
PREFIX?=/usr/local			# Base installation directory
EXEC_PREFIX?=$(PREFIX)			# Binary installation directory
BOOTDIR?=$(EXEC_PREFIX)/boot  		# Boot files directory
INCLUDEDIR?=$(PREFIX)/include 		# Header files directory

# Add kernel-specific compiler flags
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-omit-frame-pointer
# -ffreestanding: Compile for freestanding environment (no standard library)
# -Wall: Enable all common warnings
# -Wextra: Enable extra warnings
# -fno-omit-frame-pointer: Preserve frame pointers for reliable stack traces

# Add kernel-specific preprocessor flags
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
# -D__is_kernel: Define __is_kernel macro
# -Iinclude: Add include directory to include path

# Add kernel-specific linker flags and libraries
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc
# -nostdlib: Don't use standard system startup files or libraries
# -lk: Link with kernel library
# -lgcc: Link with GCC support library

# Architecture-specific directory
ARCHDIR=arch/$(HOSTARCH)

# Include architecture-specific make configuration
include $(ARCHDIR)/make.config

# Add architecture-specific flags
CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

# Define kernel object files
KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
init/kernel.o \
init/shell.o \

# Define all object files including C runtime objects
OBJS=\
$(ARCHDIR)/boot/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/boot/crtn.o \

# Define linking order (very important for C runtime)
LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/boot/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/boot/crtn.o \

# Special targets that don't create files
.PHONY: all clean install install-headers install-kernel
# File extension rules
.SUFFIXES: .o .c .nasm

# Default target
all: olympos.kernel

# Kernel binary target
olympos.kernel: $(OBJS) $(ARCHDIR)/boot/linker.ld
	$(CC) -T $(ARCHDIR)/boot/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot olympos.kernel  # Verify multiboot header

# Special rule for C runtime start/end files
$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

# Compile C source files
# -MD: Generate dependency files
# -c: Compile only, don't link
# -std=gnu11: Use GNU C11 standard
.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

# Compile NASM source files
.nasm.o:
	nasm $(NASMFLAGS) $< -o $@

# Clean target: remove all generated files
clean:
	rm -f olympos.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

# Installation targets
install: install-headers install-kernel

# Install header files
install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

# Install kernel binary
install-kernel: olympos.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp olympos.kernel $(DESTDIR)$(BOOTDIR)

# Include generated dependency files
-include $(OBJS:.o=.d)
