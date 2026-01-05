# 🍚 kacchiOS

A minimal, educational baremetal operating system designed for teaching OS fundamentals.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86-lightgrey.svg)]()

## 📖 Overview

kacchiOS is a simple, bare-metal operating system built from scratch for educational purposes. It provides a clean foundation for students to learn operating system concepts by implementing core components themselves.

### Current Features

- ✅ **Multiboot-compliant bootloader** - Boots via GRUB/QEMU
- ✅ **Serial I/O driver** (COM1) - Communication via serial port
- ✅ **Null process** - Single process that reads and echoes input
- ✅ **Basic string utilities** - Essential string operations
- ✅ **Clean, documented code** - Easy to understand and extend

### Future Extensions (Student Assignments)

Students will extend kacchiOS by implementing:
- 📝 **Memory Manager**
- 📝 **Process Manager**
- 📝 **Scheduler**

## 🚀 Quick Start

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib

# On Arch Linux
sudo pacman -S base-devel qemu gcc-multilib

# On macOS
brew install qemu i686-elf-gcc
```

### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/kacchiOS.git
cd kacchiOS

# Build the OS
make clean
make

# Run in QEMU
make run
```

You should see:
```
========================================
    kacchiOS - Minimal Baremetal OS
========================================
Hello from kacchiOS!
Running null process...

kacchiOS> 
```

Type something and press Enter - it will echo back!

## 📁 Project Structure

```
kacchiOS/
├── boot.S          # Bootloader entry point (Assembly)
├── kernel.c        # Main kernel (null process)
├── serial.c        # Serial port driver (COM1)
├── serial.h        # Serial driver interface
├── string.c        # String utility functions
├── string.h        # String utility interface
├── types.h         # Basic type definitions
├── io.h            # I/O port operations
├── link.ld         # Linker script
├── Makefile        # Build system
└── README.md       # This file
```

## 🛠️ Build System

### Makefile Targets

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build kernel.elf |
| `make run` | Run in QEMU (serial output only) |
| `make run-vga` | Run in QEMU (with VGA window) |
| `make debug` | Run in debug mode (GDB ready) |
| `make clean` | Remove build artifacts |

## 📚 Learning Resources

### Recommended Reading

- [XINU OS](https://xinu.cs.purdue.edu/) - Educational OS similar to kacchiOS
- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [The Little OS Book](https://littleosbook.github.io/) - Practical OS development
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/) - OS concepts textbook

### Related Topics

- x86 Assembly Language
- Memory Management
- Process Scheduling
- System Calls
- Interrupt Handling

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Guidelines

1. Keep code simple and educational
2. Add comments explaining complex concepts
3. Follow existing code style
4. Test changes in QEMU before submitting

## 📄 License

This project is licensed under the MIT License.

## 👨‍🏫 About

kacchiOS was created as an educational tool for teaching operating system concepts. It provides a minimal, working foundation that students can extend to learn core OS principles through hands-on implementation.

## 🙏 Acknowledgments

- Inspired by XINU OS
- Built with guidance from OSDev community
- Thanks to all students who have contributed

## 🛠️ Windows/WSL Build Notes

- Use WSL or an i686 cross-toolchain (gcc/ld/as) with make available.
- In WSL: `sudo apt-get install build-essential qemu-system-x86 gcc-multilib` then `make clean && make`.
- Run with `make run` (serial stdio). For a GUI window, use `make run-vga`.

## 🧾 Submission Checklist

- Fill docs/Checklist.md and docs/Project_Report.md, then export both to PDF as required.
- Place Demo_Video.mp4 under video/ before zipping.
- Zip with required naming: Series_XX_Section_X_Group_XX_kacchiOS.zip including src/, docs/, scheduler.c, scheduler.h, Makefile, and README.
