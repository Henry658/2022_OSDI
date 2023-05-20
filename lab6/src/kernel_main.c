#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "utils.h"
#include "sched.h"

void kernel_main(void){


	system_init();
	delay(10000000);
	uart_printf("# Hello I'm raspi3 B+ model\n");
    
   	mailbox_board_revision();
  	mailbox_vc_memory();
	int el = get_el();
	uart_printf("Exception level %d \n",el);   
	
	scheduler_init();
	
	//while(1){
		//shell_standby();
	//}
	
	thread_create(exec_thread);
	
	uart_printf("thread create done\n");
	uart_printf("run queue list size: %d\n", run_queue_list_size());
	uart_printf("zombie queue list size: %d\n", zombie_queue_list_size());
	while(1){
		schedule();
	}
}
