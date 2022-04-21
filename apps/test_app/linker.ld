MEMORY
{
    APP (rx) : ORIGIN = 0x08000000, LENGTH = (512 * 64)
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

ENTRY(start)