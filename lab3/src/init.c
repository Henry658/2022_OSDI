#include "mini_uart.h"
#include "devicetree.h"
#include "timer.h"
#include "irq.h"
#include "cpio.h"

void system_init(){
	
	init_dtb();
	uart_init();
	init_cpio();
	enable_interrupt();
	timer_list_init();
	//core_timer_init_enable();
	//fdt_traverse();
	
}




