ENTRY(start)

SECTIONS
{
    . = 512K;

    .boot :
    {
        KEEP(*(.multiboot_header))
    }

    .text :
    {
        *(.text)
    }
}