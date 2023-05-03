# Apostrof
An operating system from scratch just for fun

This is not a serious operating system.

You can contribute any time you want and I'd appreciate it. If you are having trouble with building, maybe there are some instructions missing. In that case, please let me know on Discord: `IliasHDZ#4937`

## How to build
Building works only on Linux I'm afraid. So I recommend using WSL if your are on Windows.

You'll first need GCC and binutils. If you are on an x86 based computer, you can just install it
using the package manager. Otherwise you'll have to compile GCC yourself and enable cross compilation to
i686 architecture. More info here: https://wiki.osdev.org/GCC_Cross-Compiler

Other packages required are (apt package names): `nasm`, `xorriso`, `grub-pc-bin`, `grub-common`, `cmake`

You'll need an emulator to test out the OS. You can use VirtualBox or you can install qemu (apt package name: `qemu-system-x86`)

Once all the required packages are installed, make sure CMake can find your compiler and then run the following commands:

```bash
./config.sh
./build.sh
```

The output ISO file can then be found in `bin/apostrof.iso`.

You can use VirtualBox to load the ISO file or you can use the following command to run it in qemu:

```bash
./test.sh
```

