#include "mini_uart.h"
#include "devicetree.h"
#include "timer.h"
#include "exception.h"
#include "cpio.h"
#include "buddy.h"
#include "alloc.h"

void system_init(){
	init_dtb();
	uart_init();
	init_cpio();
	buddy_init();
	enable_interrupt();
	timer_list_init();
	free_chunk_init();
	//core_timer_init_enable();
	//fdt_traverse();
}




