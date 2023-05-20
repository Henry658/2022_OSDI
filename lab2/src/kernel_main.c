#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "devicetree.h"
#include "cpio.h"


void kernel_main(void)
{	init_dtb();
	uart_init();
	delay(10000000);
    	uart_send_string("# Hello I'm raspi3 B+ model\r\n");
    
    	mailbox_board_revision();
    	mailbox_vc_memory();
   
    	
	init_cpio();
        //fdt_traverse();
	while (1) {
		shell_standby();
	}
}
