#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"

void kernel_main(void)
{
	uart_init();
	
    uart_send_string("# Hello I'm raspi3 B+ model\r\n");
    
    mailbox_board_revision();
    mailbox_vc_memory();

	while (1) {
		shell_standby();
	}
}
