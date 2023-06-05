ENTRY("_start")

KERNEL_PMA  = 0x00100000;
KERNEL_VMA  = 0xC0000000;

SECTIONS
{
    /DISCARD/ : {
        *(.note.gnu.property)
        *(.note.GNU-stack)
        *(.comment)
        *(.iplt)
    }

    . = KERNEL_PMA;
    _kernel_start = .;
    PROVIDE(KERNEL_PHYS_START = _kernel_start);

    .multiboot :
    {
        build/output/boot.o(.multiboot)
    }

    .kernel_entry_text ALIGN(4K) :
    {
        build/output/boot.o(.text)
        build/output/boot.o(.bss)
    }
    _kernel_entry_end = .;

    . += KERNEL_VMA;

    /*.kernel_text ALIGN (4K) : AT(ADDR(.kernel_text) - KERNEL_VMA + _kernel_entry_end)*/
    .kernel_text ALIGN (4K) : AT(ADDR(.kernel_text) - KERNEL_VMA)
    {
        build/output/kernel/*.o(.text)
        build/output/isr.o(.text)
        arch/x86/kernel/*.o(.text)
    }

    .kernel_data ALIGN (4K) : 
    {
        arch/x86/kernel/*.o(.data)
    	build/output/kernel/*.o(.data)
    	build/output/arch/x86/kernel/*.o(.data)
    	build/output/arch/x86/boot/*.o(.data)
    }

    .kernel_rodata ALIGN (4K) :
    {
        arch/x86/kernel/*.o(.rodata)
	    build/output/kernel/*.o(.rodata)
        build/output/arch/x86/kernel/*.o(.rodata)
    	build/output/arch/x86/boot/*.o(.rodata)

    }

    .kernel_bss ALIGN (4K) :
    {
    	build/output/kernel/*.o(.bss)
        arch/x86/kernel/*.o(.bss)
        build/output/arch/x86/kernel/*.o(.bss)
    	build/output/arch/x86/boot/*.o(.bss)

    }

    _kernel_end = . - KERNEL_VMA;
    _kernel_virt_end = .;
    PROVIDE(KERNEL_PHYS_END = _kernel_end);
    PROVIDE(KERNEL_VIRT_END = _kernel_virt_end);

}
