mkdir build
mkdir dist

mkdir build/boot
mkdir build/kernel
mkdir build/kernel/int
mkdir build/kernel/drv

gcc -c -m32 -ffreestanding -o build/kernel/main.o src/kernel/main.c
gcc -c -m32 -ffreestanding -o build/kernel/vga.o src/kernel/vga.c
gcc -c -m32 -ffreestanding -o build/kernel/int/isr.o src/kernel/int/isr.c
gcc -c -m32 -ffreestanding -o build/kernel/int/idt.o src/kernel/int/idt.c
gcc -c -m32 -ffreestanding -o build/kernel/drv/timer.o src/kernel/drv/timer.c

nasm -f elf32 -o build/boot/header.o src/boot/header.asm
nasm -f elf32 -o build/boot/main.o src/boot/main.asm
nasm -f elf32 -o build/kernel/int/isr_a.o src/kernel/int/isr_a.asm

ld -m elf_i386 -n -o dist/kernel.bin -T linker.ld build/boot/header.o build/boot/main.o build/kernel/int/isr_a.o build/kernel/main.o build/kernel/vga.o build/kernel/int/isr.o build/kernel/int/idt.o build/kernel/drv/timer.o
cp dist/kernel.bin iso/boot/kernel.bin

grub-mkrescue /usr/lib/grub/i386-pc -o dist/os-test.iso iso