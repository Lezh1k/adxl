/* Linker script to configure memory regions. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}

/*GROUP(libgcc.a libc.a libm.a libnosys.a)*/
ENTRY(ResetIntHandler)

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.after_vectors*)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);

        PROVIDE(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE(__init_array_end = .);

        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data :
    {
        . = ALIGN(4) ;
        _data = .;
        *(.data*)
        . = ALIGN(4) ;
        _edata = .;
    } > RAM AT > FLASH

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RAM

    PROVIDE(_pvHeapStart = .);
    PROVIDE(__stack_top = ORIGIN(RAM) + LENGTH(RAM));
}
