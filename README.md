# XylenOS
 XylenOS (from Greek "ξυλοσ"), started as UselessOS or uOS, is a hobbyist project made by a Turkish and Bangladeshi Hobbyist. <br/>


## Screenshots
![Screenshot of XylenOS 0.2.5](https://media.discordapp.net/attachments/1323392171428151307/1379860495313408052/image.png?ex=6841c6ba&is=6840753a&hm=5fa9ad5105eba77b947f3460107fe8fe29fe8f92b7e67f72a59d044f201306f6&=&format=webp&quality=lossless)


## Features

 - 32-bit with support for 64-bit hardware
 - Simple Text-Based Userland aka Shell
 - Simple Commands like touch, time, ls, cat, cp, rm
 - Fully fledged Filesystem called ZadFS™ 😎
 - Text Editor 
 - Working Speaker Drivers

## To-Do list (Roadmap of May/June)

 - [x] Fixed Bootloader
 - [x] Get Kernel to load into memory
 - [x] Text-based Userland aka Shell
 - [x] Live Clock
 - [x] Filesystem
 - [X] QoL Improvements 
 - [X] FS Commands
 - [X] Extended Filesystem
 - [X] Text Editor (W.I.P)
 - [ ] Filesystem inside the Image for Elf Parsing
 - [ ] Executable Loader (.elf) 
 - [ ] Text-User Interface
 - [ ] Mount Tool
 - 2nd Dev Phase
 - [ ] Simplification of Installation onto Hard Drive
 - [ ] Beeper Application
 - [ ] Working NAT
 - [ ] Brainfuck Interpreter
 - [ ] Package Manager
 
## Build Guide
Make sure you have `x86_64-elf-gcc` and `x86_64-elf-binutils` or the i686 equivalent installed.

Here is a simple Build script.
````
#!/bin/bash
set -e

# XylenOS Enhanced Build Script
# This builds the complete operating system with all improvements
# Author: PS2Comrade
# Date: 2025-06-07

echo "=========================================="
echo "    XylenOS 0.3.5 Enhanced Build"
echo "=========================================="

# Colors for better output (if terminal supports it)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required tools are installed
print_status "Checking build dependencies..."

MISSING_TOOLS=""

if ! command -v nasm &> /dev/null; then
    MISSING_TOOLS="$MISSING_TOOLS nasm"
fi

if ! command -v i686-elf-gcc &> /dev/null; then
    MISSING_TOOLS="$MISSING_TOOLS i686-elf-gcc"
fi

if ! command -v i686-elf-ld &> /dev/null; then
    MISSING_TOOLS="$MISSING_TOOLS i686-elf-ld"
fi

if ! command -v i686-elf-objcopy &> /dev/null; then
    MISSING_TOOLS="$MISSING_TOOLS i686-elf-objcopy"
fi

if [ ! -z "$MISSING_TOOLS" ]; then
    print_error "Missing required tools:$MISSING_TOOLS"
    echo "Please install the missing tools and try again."
    echo "On Ubuntu/Debian: sudo apt install nasm gcc-multilib"
    echo "For cross-compiler: You'll need to build i686-elf-gcc manually or use a package manager like Homebrew"
    exit 1
fi

print_success "All build tools found!"

# Clean up any previous build artifacts
print_status "Cleaning previous build artifacts..."
rm -f *.bin *.o *.elf os-image 2>/dev/null || true

# Create object directory for better organization
mkdir -p obj

# Compiler flags
CFLAGS="-m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -O2 -Wall -Wextra -Wno-unused-parameter"
LDFLAGS="-m elf_i386 -Ttext 0x1000 -e _start"

print_status "Building bootloader..."
nasm bootloader.asm -f bin -o bootloader.bin
if [ $? -eq 0 ]; then
    print_success "Bootloader assembled successfully"
else
    print_error "Bootloader assembly failed"
    exit 1
fi

print_status "Building kernel entry point..."
nasm enter_kernel.asm -f elf -o obj/enter_kernel.o
if [ $? -eq 0 ]; then
    print_success "Kernel entry point assembled"
else
    print_error "Kernel entry assembly failed"
    exit 1
fi

# Compile all C source files
print_status "Compiling C source files..."

# Core kernel files
print_status "  - Compiling kernel/main.c..."
i686-elf-gcc $CFLAGS -c kernel/main.c -o obj/kernel_main.o

print_status "  - Compiling kernel/shell.c..."
i686-elf-gcc $CFLAGS -c kernel/shell.c -o obj/shell.o

print_status "  - Compiling kernel/errors.c..."
i686-elf-gcc $CFLAGS -c kernel/errors.c -o obj/errors.o

# Library files
print_status "  - Compiling lib/string.c..."
i686-elf-gcc $CFLAGS -c lib/string.c -o obj/string.o

print_status "  - Compiling lib/memory.c..."
i686-elf-gcc $CFLAGS -c lib/memory.c -o obj/memory.o

print_status "  - Compiling lib/input.c..."
i686-elf-gcc $CFLAGS -c lib/input.c -o obj/input.o

print_status "  - Compiling lib/heap.c..."
i686-elf-gcc $CFLAGS -c lib/heap.c -o obj/heap.o

# Driver files
print_status "  - Compiling drivers/keyboard.c..."
i686-elf-gcc $CFLAGS -c drivers/keyboard.c -o obj/keyboard.o

print_status "  - Compiling drivers/vga.c..."
i686-elf-gcc $CFLAGS -c drivers/vga.c -o obj/vga.o

print_status "  - Compiling drivers/cmos.c..."
i686-elf-gcc $CFLAGS -c drivers/cmos.c -o obj/cmos.o

print_status "  - Compiling drivers/ata.c..."
i686-elf-gcc $CFLAGS -c drivers/ata.c -o obj/ata.o

print_status "  - Compiling drivers/hal.c..."
i686-elf-gcc $CFLAGS -c drivers/hal.c -o obj/hal.o

print_status "  - Compiling drivers/speaker.c..."
i686-elf-gcc $CFLAGS -c drivers/speaker.c -o obj/speaker.o

# System files
print_status "  - Compiling system/reboot.c..."
i686-elf-gcc $CFLAGS -c system/reboot.c -o obj/reboot.o

# Filesystem
print_status "  - Compiling fs/zadfs.c..."
i686-elf-gcc $CFLAGS -c fs/zadfs.c -o obj/zadfs.o

# Application files
print_status "  - Compiling apps/editor.c..."
i686-elf-gcc $CFLAGS -c apps/editor.c -o obj/editor.o

print_success "All C files compiled successfully!"

# Link all object files together
print_status "Linking kernel..."
i686-elf-ld $LDFLAGS -o kernel.elf \
    obj/enter_kernel.o \
    obj/kernel_main.o \
    obj/shell.o \
    obj/errors.o \
    obj/string.o \
    obj/memory.o \
    obj/input.o \
    obj/heap.o \
    obj/keyboard.o \
    obj/vga.o \
    obj/cmos.o \
    obj/ata.o \
    obj/hal.o \
    obj/speaker.o \
    obj/reboot.o \
    obj/zadfs.o \
    obj/editor.o

if [ $? -eq 0 ]; then
    print_success "Kernel linked successfully"
else
    print_error "Kernel linking failed"
    exit 1
fi

# Convert ELF to flat binary
print_status "Converting kernel to binary format..."
i686-elf-objcopy -O binary kernel.elf kernel.bin

# Calculate kernel size and sectors needed
size_bytes=$(stat -c%s kernel.bin 2>/dev/null || stat -f%z kernel.bin 2>/dev/null)
sectors=$(( (size_bytes + 511) / 512 ))

print_status "Kernel size: $size_bytes bytes"
print_status "Sectors needed: $sectors (each sector = 512 bytes)"

# Check if kernel size matches what bootloader expects
EXPECTED_SECTORS=21
if [ $sectors -gt $EXPECTED_SECTORS ]; then
    print_warning "Kernel is larger than expected!"
    print_warning "Current: $sectors sectors, Expected: $EXPECTED_SECTORS sectors"
    print_warning "You may need to update the bootloader disk_load call"
    print_warning "In bootloader.asm, change 'mov dh, 21' to 'mov dh, $sectors'"
else
    print_success "Kernel size is within expected limits"
fi

# Combine bootloader and kernel into final OS image
print_status "Creating final OS image..."
cat bootloader.bin kernel.bin > os-image

# Verify the final image
final_size=$(stat -c%s os-image 2>/dev/null || stat -f%z os-image 2>/dev/null)
print_status "Final OS image size: $final_size bytes"

# Clean up intermediate files (optional - comment out if you want to keep them for debugging)
print_status "Cleaning up intermediate files..."
rm -f bootloader.bin kernel.elf kernel.bin
rm -rf obj/

print_success "Build completed successfully!"
echo ""
echo "=========================================="
echo "           BUILD SUMMARY"
echo "=========================================="
echo "Output file: os-image"
echo "Total size: $final_size bytes"
echo "Kernel sectors: $sectors"
echo ""
echo "To test your OS:"
echo "  qemu-system-i386 -fda os-image"
echo ""
echo "To write to USB/floppy:"
echo "  sudo dd if=os-image of=/dev/sdX bs=512"
echo "  (Replace /dev/sdX with your device)"
echo ""
echo "To create a virtual hard drive for testing:"
echo "  qemu-img create -f raw hdd.img 10M"
echo "  qemu-system-i386 -fda os-image -hda hdd.img"
echo ""
print_success "Happy hacking with XylenOS 0.3.5!"

````
