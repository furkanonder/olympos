# Olympos
An experimental 32-bit Operating System.

## Prerequisites

Before building Olympos, you need to have the following tools installed:

### Required Packages

For Arch Linux:
```bash
# Core build tools
sudo pacman -S gcc make nasm

# For creating and running ISO
sudo pacman -S mtools xorriso grub qemu
```

For other distributions, install the equivalent packages using your package manager.

### Cross-Compiler
You need a cross-compiler targeting i686-elf.

#### For Arch Linux Users
The cross-compiler can be easily installed from the AUR

```bash
yay -S i686-elf-gcc
```

#### For Other Distributions
If you're not using Arch Linux, you'll need to build the cross-compiler from source or find equivalent packages for your distribution.

## Building

1. Clone the repository:
```bash
git clone <repository-url>
cd olympos
```

2. Build the system:
```bash
./build.sh
```

3. Create a bootable ISO:
```bash
./iso.sh
```

## Running

To run Olympos in QEMU:
```bash
./qemu.sh
```

## Build System Scripts

- `build.sh`: Main build script
- `clean.sh`: Removes all built files
- `config.sh`: Configure build environment
- `headers.sh`: Install system headers
- `iso.sh`: Create bootable ISO
- `qemu.sh`: Run OS in QEMU

## License

The GNU General Public License v3.0

