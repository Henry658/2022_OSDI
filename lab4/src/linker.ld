__va_size__ = DEFINED(__va_size__) ? __va_size__ : 0x1000;
__simple_alloc_size__ = DEFINED(__simple_alloc_size__) ? __simple_alloc_size__ : 0x80000;

SECTIONS
{	. = 0x80000;
    _begin = .;
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	. = ALIGN(0x8);
	bss_begin = .;
	.bss : { *(.bss*) } 
	bss_end = .;
	
	_va_start = .;
	
	. = . + __va_size__;
	. = ALIGN(0x8);
	_va_end = .;
	
	_alloc_start = .;
	. = . + __simple_alloc_size__;
	. = ALIGN(0x8);
	_alloc_end = .;
	
	_end = .;
}

__kernel_size = (_end - _begin);
