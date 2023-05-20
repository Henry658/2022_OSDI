#define __ASSEMBLY__
#include "mm.h"
#include "mmu.h"
#include "sysreg.h"

.section ".text.boot"

.globl _start
_start:
	mov	x11, x0 //dtb address
		
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang:
	b 	proc_hang

master:
	
	bl 	from_el2_to_el1
	
	
from_el2_to_el1:
	// disable MMU
	ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0         // SCTLR_EL1, System Control Register (EL1)
	
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
	
	// load exception_table to VBAR_EL1
	ldr  x0, =vectors            // load VBAR_EL1 with virtual
	msr  vbar_el1, x0           // vector table address
	
	// mask all interrupt, and set exception level to el0
	ldr	x0, =SPSR_EL1_VALUE
	msr	spsr_el1, x0
	
	// set stack pointer
	ldr	x0, =0x60000
	mov	sp, x0
	
	// setup TCR
	ldr     x0, =TCR_EL1_VALUE
	msr     tcr_el1, x0
	
	// setup MAIR_EL1
	ldr     x0, =MAIR_EL1_VALUE
	msr     mair_el1, x0
	
	// create page table
	bl      page_table_create
	
	// enable MMU
	ldr     x1, =SCTLR_VALUE_MMU_ENABLE
	mrs     x2, sctlr_el1
	orr     x2, x2, x1        // bitwise or
	msr     sctlr_el1, x2     // enable MMU
	
	// clear bss 
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	//mov sp to virtual address
	ldr     x1, = KERNEL_VIRTURE_BASE
	add     sp, sp, x1
	
	// indirect branch
	ldr     x0, =kernel_main  // indirect branch to the virtual address
	br      x0
	
	b 	proc_hang         // should never come here
	
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

page_table_create:

	// Each page table base address
	// x0: PGD (1 page, 1 entry)
	// x1: PUD (1 page, 2 entry)
	// x2: PMD (1 page, 512 entry)
	// x3: PTE (512 page, each 512 entry)
/*	
`+---------------------------------------------------------------------------------------+
 |0\          |0\1\        |0\1\ ....\511|0\ ... \511|0\ ... \511|   ...     |0\ ... \511|
 +------------+------------+-------------+-----------+-----------------------+-----------+
 PGD          PUD          PMD           PTE#0       PTE#1           ...     PTE#511
 x0           x1           x2            x3
              0 for normal               #0 ~ #503 for normal PTE         
              1 for device               
*/	
	
	// omit first 16 bit (using physical address)
	ldr     x0, =page_dir
	lsl     x0, x0, #16
	lsr     x0, x0, #16            // PGD address
	add     x1, x0, #PAGE_SIZE     // PUD address
	add     x2, x1, #PAGE_SIZE     // PMD address
	add     x3, x2, #PAGE_SIZE     // PTE address
	
	// setup PGD
	ldr     x4, =PGD0_ATTR
	orr     x4, x1, x4 // PUD physical address
	str     x4, [x0]
	
	// setup PUD0    for normal memory
	ldr     x4, =PUD0_ATTR
	orr     x4, x2, x4 // PMD physical address
	str     x4, [x1]
	
	// setup PUD1    for device memory
	ldr     x4, =PUD1_ATTR
	mov     x5, 0x40000000  //(1GB section start from 0x40000000)
        //The second one maps 0x40000000 - 0x7fffffff(ARM local peripherals).
	orr     x4, x5, x4 // PMD physical address
	str     x4, [x1, #8]
	
	// setup PMD (512 entry in 1 page)
	mov     x4, x3    // x4 = point to the current PTE address
	mov     x5, xzr   // x5 = loop iter
	mov     x6, #512  // x6 = loop size
	
1:      
        ldr     x7, =PMD0_ATTR
        orr     x7, x4, x7 // PTE physical address
        str     x7, [x2, x5, lsl #3] // x2 + x5 * 8(<<3)
        add     x5, x5, #1           // x5 = loop iter
        add     x4, x4, #PAGE_SIZE   // x4 = point to the current PTE address 
	cmp     x5, x6
	b.ls    1b
	
	// setup normal PTE (512 * 512 - 4096 = 258048 entry)
	mov     x4, xzr // physical address
	mov     x5, xzr // i = 0
	mov     x6, #258048
2:
       ldr      x7, =PTE_NORAL_ATTR
       orr      x7, x4, x7
       str      x7, [x3, x5, lsl #3] // x3 + x5 * 8(<<3)
       add      x5, x5, #1
       add      x4, x4, #PAGE_SIZE
       cmp      x5, x6
       b.ls     2b
       
       // setup device PTE (16MB = 4096 entry) for peripheral region
       add	x6, x6, #4096 // 262144
3:
	ldr	x7, =PTE_DEVICE_ATTR
	orr	x7, x4, x7
	str	x7, [x3, x5, lsl #3] // x3 + x5 * 8(<<3)
	add	x5, x5, #1
	add	x4, x4, #PAGE_SIZE
	cmp	x5, x6
	b.ls	3b
      
       msr      ttbr0_el1, x0 // load PGD to the buttom translation based register.
       msr      ttbr1_el1, x0 // load PGD to the upper translation based register.
       
       ret
       
	
