.global cpu_switch_to
cpu_switch_to:
	mov  x9,  sp
	stp x19, x20, [x0, 16 * 0]
	stp x21, x22, [x0, 16 * 1]
	stp x23, x24, [x0, 16 * 2]
	stp x25, x26, [x0, 16 * 3]
	stp x27, x28, [x0, 16 * 4]
	stp  fp,  lr, [x0, 16 * 5]
	str  x9,      [x0, 16 * 6]
	
	ldp x19, x20, [x1, 16 * 0]
	ldp x21, x22, [x1, 16 * 1]
	ldp x23, x24, [x1, 16 * 2]
	ldp x25, x26, [x1, 16 * 3]
	ldp x27, x28, [x1, 16 * 4]
	ldp  fp,  lr, [x1, 16 * 5]
	ldr  x9,      [x1, 16 * 6]
	mov  sp,  x9
	ret

.global store_context
store_context:
	mov  x9,  sp
	stp x19, x20, [x0, 16 * 0]
	stp x21, x22, [x0, 16 * 1]
	stp x23, x24, [x0, 16 * 2]
	stp x25, x26, [x0, 16 * 3]
	stp x27, x28, [x0, 16 * 4]
	stp  fp,  lr, [x0, 16 * 5]
	str  x9,      [x0, 16 * 6]
	ret

.global restore_context
restore_context:
	ldp x19, x20, [x1, 16 * 0]
	ldp x21, x22, [x1, 16 * 1]
	ldp x23, x24, [x1, 16 * 2]
	ldp x25, x26, [x1, 16 * 3]
	ldp x27, x28, [x1, 16 * 4]
	ldp  fp,  lr, [x1, 16 * 5]
	ldr  x9,      [x1, 16 * 6]
	mov  sp,  x9
	ret


//tpidr_el1 saved the current thread's data structure

.global set_current_task
set_current_task:
	msr tpidr_el1, x0
	ret

.global get_current_task
get_current_task:
	mrs x0, tpidr_el1
	ret

.global update_pgd
update_pgd:
    dsb ish  // ensure write has completed
    msr ttbr0_el1, x0  // switch translation based address.
    tlbi vmalle1is  // invalidate all TLB entries
    dsb ish  // ensure completion of TLB invalidatation
    isb  // clear pipeline
    ret
