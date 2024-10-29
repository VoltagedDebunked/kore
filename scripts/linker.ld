OUTPUT_FORMAT("elf32-i386")

MEMORY
{
    ROM (rx) : ORIGIN = 0x0000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x100000, LENGTH = 10M
}

SECTIONS
{
    . = ORIGIN(ROM);

    .text : {
        *(.text)
    } > ROM

    .data : {
        *(.data)
    } > RAM

    .bss : {
        *(.bss)
    } > RAM
}
