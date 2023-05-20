#include "typedef.h"

#ifndef EXCEPTION_H
#define EXCEPTION_H

#define INTR_STACK_SIZE 4096
#define INTR_STACK_TOP_IDX (INTR_STACK_SIZE - 16) // sp need 

typedef struct trapframe{
	uint64_t x[31];
	uint64_t sp_el0;
	uint64_t elr_el1;
	uint64_t spsr_el1;
}trapframe_t;

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address);

void not_implemented();

void handle_sync(unsigned long esr, unsigned long elr, trapframe_t * trapframe);

void sys_call_router(uint64_t syscall_num, trapframe_t * trapframe);

void irq_stk_switcher();

void handle_irq(void);

void irq_return();

void enable_interrupt();

void disable_interrupt();

static unsigned long long lock_count = 0;

void lock();

void unlock();

#endif
