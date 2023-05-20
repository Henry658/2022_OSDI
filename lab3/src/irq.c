#include "utils.h"
#include "mini_uart.h"
#include "timer.h"
#include "entry.h"
#include "peripherals/irq.h"
#include "peripherals/timer.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",
	"FIQ_INVALID_EL1t",
	"ERROR_INVALID_EL1t",

	"SYNC_INVALID_EL1h",
	"IRQ_INVALID_EL1h",
	"FIQ_INVALID_EL1h",
	"ERROR_INVALID_EL1h",

	"SYNC_INVALID_EL0_64",
	"IRQ_INVALID_EL0_64",
	"FIQ_INVALID_EL0_64",
	"ERROR_INVALID_EL0_64",

	"SYNC_INVALID_EL0_32",
	"IRQ_INVALID_EL0_32",
	"FIQ_INVALID_EL0_32",
	"ERROR_INVALID_EL0_32"
};

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address){
	uart_printf("%s, ESR: %x, address: %x\r\n",entry_error_messages[type], esr, address);
	uart_printf("Exception class (EC) 0x%x\n",(esr >> 26) & 0b111111);
	uart_printf("Instruction specific syndrome (ISS) 0x%x\n", esr & 0xFFFFFF);
}



void not_implemented(){
	uart_printf("Not implemented function.... \n");
	while(1);
}

void handle_sync(void){
	
	unsigned long long spsr_el1, elr_el1, esr_el1;
	__asm__ __volatile__("mrs %0, spsr_el1 \n\t" : "=r" (spsr_el1));
	__asm__ __volatile__("mrs %0, elr_el1 \n\t" : "=r" (elr_el1));
	__asm__ __volatile__("mrs %0, esr_el1 \n\t" : "=r" (esr_el1));
	uart_printf("spsr_el1: %x, elr_el1: %x, esr_el1: %x\n", spsr_el1, elr_el1, esr_el1);
}


void handle_irq(void){
	if(*CORE0_INTERRUPT_SOURCE & SYSTEM_TIMER_IRQ_1){
		core_timer_disable();
		core_timer_handler();
		core_timer_enable();
	}
}

void enable_interrupt(){
	__asm__ __volatile__("msr daifclr, 0xf");
}

void disable_interrupt(){
	__asm__ __volatile__("msr daifset, 0xf");
}
