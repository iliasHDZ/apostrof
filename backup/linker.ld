MEMORY
{
    KERNEL (rx)     : ORIGIN = 0x009000, LENGTH = (512 * 255)
    BSS (rx)        : ORIGIN = 0x028E00, LENGTH = (512 * 512)
}

SECTIONS
{
    .text :
    {
        *(.kernel_header)
        *(.text)
        *(.rodata)
        *(.data)
        *(.note.gnu.property)
        *(.eh_frame)
        *(.kernel_footer)

        FILL(0x00);
        . = ORIGIN(KERNEL) + LENGTH(KERNEL);
    } >KERNEL

    .bss :
    {
        . = 0;
    } >BSS
}

OUTPUT_FORMAT(binary)