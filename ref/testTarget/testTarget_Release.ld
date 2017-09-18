/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2017
 * Generated linker script file for LPC824
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.23
 * LPCXpresso v8.2.2 [Build 650] [2016-09-09]  on 14.08.2017 15:44:46
 */

INCLUDE "testTarget_Release_library.ld"
INCLUDE "testTarget_Release_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */
	
	
	/*.text : ALIGN(4)
    {
        FILL(0xff)
        . = ALIGN(0x100) ; 
		KEEP(*(.isr_vector))
    } 
	*/
	
    .text : ALIGN(4)
    {
        FILL(0xff)
        __bugaga = . ;
        __vectors_start__ = ABSOLUTE(.) ;
/*	KEEP(*(.isr_vector))
        . = ALIGN(4) ;

        . = __bugaga + 0x2000 ;
*/
        __vectors_start2__ = ABSOLUTE(.) ;
		
        KEEP(*(.isr_vector2))
        /* Global Section Table */
        . = ALIGN(4) ; 
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
	    /* End of Global Section Table */

        *(.after_vectors*)

    } >MFlash32

    .text : ALIGN(4)    
    {
        *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
			/* C++ constructors etc */
			. = ALIGN(4);
			KEEP(*(.init))
			
			. = ALIGN(4);
			__preinit_array_start = .;
			KEEP (*(.preinit_array))
			__preinit_array_end = .;
			
			. = ALIGN(4);
			__init_array_start = .;
			KEEP (*(SORT(.init_array.*)))
			KEEP (*(.init_array))
			__init_array_end = .;
			
			KEEP(*(.fini));
			
			. = ALIGN(4);
			KEEP (*crtbegin.o(.ctors))
			KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
			KEEP (*(SORT(.ctors.*)))
			KEEP (*crtend.o(.ctors))
			
			. = ALIGN(4);
			KEEP (*crtbegin.o(.dtors))
			KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
			KEEP (*(SORT(.dtors.*)))
			KEEP (*crtend.o(.dtors))
			. = ALIGN(4);
			/* End C++ */
    } > MFlash32
    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4) 
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > MFlash32
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > MFlash32
    __exidx_end = .;

    _etext = .;
        
    /* MAIN DATA SECTION */
        /* Default MTB section */
        .mtb_buffer_default (NOLOAD) :
        {
           KEEP(*(.mtb*))
        } > RamLoc8
    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamLoc8
    /* Main DATA section (RamLoc8) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > RamLoc8 AT>MFlash32
    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamLoc8
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamLoc8

    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamLoc8 - 0);

    /* ## Create checksum value (used in startup) ## */
    PROVIDE(__valid_user_code_checksum = 0 - 
                                         (_vStackTop 
                                         + (ResetISR + 1) 
                                         + (( DEFINED(NMI_Handler) ? NMI_Handler : M0_NMI_Handler ) + 1) 
                                         + (( DEFINED(HardFault_Handler) ? HardFault_Handler : M0_HardFault_Handler ) + 1) 
                                         )
           );
}
