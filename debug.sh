make && qemu-system-i386 -gdb tcp::9000 -S -debugcon stdio -m 4G -vga std -drive id=disk,file=image.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -drive format=raw,file=dist/apostrof.iso
