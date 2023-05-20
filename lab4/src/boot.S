#include "mm.h"
#include "sysreg.h"

.section ".text.boot"

.globl _start
_start:	
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	
	mov	sp, #LOW_MEMORY 
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero
	
	bl 	from_el2_to_el1
	
	
from_el2_to_el1:
	// disable MMU
	ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0
	
	// make el0, el1 can use Floating point and Advanced SIMD
	ldr	x0, =CPACR_EL1_VALUE
	msr	CPACR_EL1, x0
	
	// set AArch64 for el2
	ldr	x0, =HCR_EL2_VALUE
	msr	hcr_el2, x0
	
	// mask all interrupt, and set interrupt level to el1h
	ldr	x0, =SPSR_EL2_VALUE
	msr	spsr_el2, x0
	
	adr 	x0, el1_entry
	msr	elr_el2, x0 
	
	eret
	
el1_entry:
	// set stack pointer
	ldr	x0, =0x60000
	mov	sp, x0
	
	// load exception_table to VBAR_EL1
	bl irq_vector_init
	/*
	adr	x0, vectors
	msr	vbar_el1, x0
	*/
	
	// mask all interript, and set exception level to el0
	ldr	x0, =SPSR_EL1_VALUE
	msr	spsr_el1, x0
	
	//adr	x0, el0_entry
	//msr	elr_el1, lr
	
	bl	kernel_main
	b 	proc_hang	
.globl el0_entry
el0_entry:
	//set stack pointer
	ldr	x0, =0x40000 //0x40000
	mov	sp, x0
	//bl	test

	bl	get_el_value
	//bl enable_irq
	//ret
	// should never come here

