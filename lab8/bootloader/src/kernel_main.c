#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "utils.h"
#include "loader.h"

void kernel_main(void)
{
	uart_init();
	
	
	//delay(3000000000);
	uart_send_string("# Hello I'm raspi3 B+ model\r\n");
	uart_send_string("# Hello I'm bootloader\r\n");
	uart_send_string("# Please sending the kernel image from host\r\n");
	mailbox_board_revision();
	mailbox_vc_memory();

	loading();
}
