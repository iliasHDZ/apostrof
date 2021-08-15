MEMORY
{
    APP (rx) : ORIGIN = 0x04000000, LENGTH = (512 * 64)
}

SECTIONS
{
    .text :
    {
        *(.header)
        *(.text)
        *(.rodata)
        *(.data)
        *(.eh_frame)
    } >APP

    .bss :
    {
        *(.bss)
    } >APP

    .fill :
    {
        *(.note.gnu.property)
    } >APP
}

OUTPUT_FORMAT(binary)