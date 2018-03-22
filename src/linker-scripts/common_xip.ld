/* Define the entry point of the output file. */
ENTRY(os_reset_handler)

/* Define sections. */
SECTIONS
{
    .init_vtable : {
        LONG(ORIGIN(ram) + LENGTH(ram)) /* Initial stack pointer */
        LONG(ADDR(.reset) | 1) /* Reset Handler address */
    } >rom

    . = ALIGN(4);

    .reset : ALIGN(16) {
        KEEP(*(.reset*)) /* Reset handler */
    }

    .text : {
        *(.text*)   /* Program code */
        . = ALIGN(4);
        *(.rodata*) /* Read-only data */
        . = ALIGN(4);
        KEEP (*(.init))
        KEEP (*(.fini))
    } >rom

    /* C++ Static constructors/destructors, also used for __attribute__
     * ((constructor)) and the likes */
    .preinit_array : {
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;
    } >rom
    .init_array : {
        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    } >rom
    .fini_array : {
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;
    } >rom

    .data : {
        _data = .;
        *(.data*)   /* Read-write initialized data */
        . = ALIGN(4);
        _edata = .;
    } >ram AT >rom
    _data_loadaddr = LOADADDR(.data);

    .bss : {
        _bss = .;
        *(.bss*)    /* Read-write zero initialized data */
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >ram

    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    PROVIDE ( __end__ = _ebss );
}
