__va_size__ = DEFINED(__va_size__) ? __va_size__ : 0x4000;
__simple_alloc_size__ = DEFINED(__simple_alloc_size__) ? __simple_alloc_size__ : 0x1000000;

SECTIONS
{
	. = 0xFFFF000000000000;
	. += 0x80000;            

    	kernel_start = .;
    	
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	
	. = ALIGN(0x10);
	bss_begin = .;
	.bss : { *(.bss*) } 
	. = ALIGN(0x10);
	bss_end = .;
	
	_va_start = .;
	. = . + __va_size__;
	. = ALIGN(0x10);
	_va_end = .;
	
	_alloc_start = .;
	. = . + __simple_alloc_size__;
	. = ALIGN(0x10);
	_alloc_end = .;
	
	. = ALIGN(0x1000);
	page_dir = .;
	.data.pgd :
	{
	. += (515 * (1 << 12)); /* (PGD * 1) + (PUD * 1) + (PMD * 1) + (PTE * 512) */
	}
	. = ALIGN(0x1000);
	
	kernel_end = .;
}

__kernel_size = (kernel_end - kernel_start);
