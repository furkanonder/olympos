# Get the host triplet from default-host.sh script
DEFAULT_HOST!=../default-host.sh
# Allow override of host, otherwise use default
HOST?=DEFAULT_HOST
# Extract architecture from host triplet (e.g., i386-elf -> i386)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

# Default compiler and build flags
CFLAGS?=-O2 -g		# Optimization level 2 and debugging info
CPPFLAGS?=		# C preprocessor flags
LDFLAGS?=		# Linker flags
LIBS?=              	# Libraries to link against

# Installation directories
DESTDIR?=           		# For staged installations
PREFIX?=/usr/local		# Base installation directory
EXEC_PREFIX?=$(PREFIX)		# For architecture-dependent files
INCLUDEDIR?=$(PREFIX)/include	# Header files location
LIBDIR?=$(EXEC_PREFIX)/lib	# Library files location

# Set up C flags for freestanding environment
CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
# Add include directory to preprocessor flags and define __is_libc
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude

# Kernel-specific versions of flags (for libk)
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk  # Define __is_libk for kernel library

# Architecture-specific directory
ARCHDIR=arch/$(HOSTARCH)

# Include architecture-specific configurations
include $(ARCHDIR)/make.config

# Add architecture-specific flags to both libc and libk
CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

# Freestanding objects (don't require hosted environment)
FREEOBJS=\
$(ARCH_FREEOBJS) \
assert/assert.o \
stdio/printf.o \
stdio/putchar.o \
stdio/getchar.o \
stdio/puts.o \
stdio/snprintf.o \
stdio/vsnprintf.o \
stdlib/abort.o \
stdlib/itoa.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strchr.o \
string/strcmp.o \
string/strcspn.o \
string/strlen.o \
string/strspn.o \
string/strtok.o \
sys/syscall.o \

# Objects that require a hosted environment
HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

# Combine all objects
OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

# Create list of kernel library objects by transforming .o to .libk.o
LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

# Define the libraries we're building
#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a

# Special targets and suffix rules
.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S

# Default target
all: $(BINARIES)

# Rule to build the C library
libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

# Rule to build the kernel library
libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

# Rules for building object files
.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.c.S:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

# Rules for building kernel library object files
.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

# Clean target - remove all generated files
clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

# Installation targets
install: install-headers install-libs

# Install headers to system root
install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

# Install libraries to system root
install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

# Include dependency files if they exist
-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
