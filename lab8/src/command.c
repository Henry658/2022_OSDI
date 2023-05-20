#include "mini_uart.h"
#include "reboot.h"
#include "cpio.h"
#include "test.h"
#include "devicetree.h"
#include "buddy.h"
#include "string.h"
 
void input_buffer_overflow_message(char *str){
    uart_send_string("input buffer overflow message\n");
}
void command_help(){
    uart_send_string("\r\n");
    uart_send_string("# help\r\n");
    uart_send_string("help\t: print this help menu\r\n");
    uart_send_string("hello\t: print Hello World!\r\n");
    uart_send_string("reboot\t: reboot the device\r\n");
    uart_send_string("cancel reboot\t: cancel reboot\r\n");
    uart_send_string("ls\t: list\r\n");
    uart_send_string("cat\t: cat\r\n");
    uart_send_string("test alloc\t: test alloc\r\n");
    uart_send_string("fdt_traverse\t: fdt_traverse\r\n");
    uart_send_string("exec\t: exec FILENAME\r\n");
    uart_send_string("settimeout\t: settimeout MESSAGE SECONDS\r\n");
    //uart_printf("test\t: test buddy print\n");
    //uart_printf("test1\t; test dynamic alloc\n");
	uart_send_string("\r\n");
}

void command_hello(){
    uart_send_string("Hello I'm kernel!\r\n");
}
void command_reboot(){
    uart_send_string("rebooting...\r\n");
    reset();
}

void command_cancel_reboot(){
    uart_send_string("cancel reboot\r\n");
    cancel_reset();
}

void command_not_found(){
    uart_send_string("shell: command not found\r\n");
}

void command_ls(){
    ls();
}

void command_cat(char *str){
    char * filename;
    for(int i = 0 ; i < strlen(str+4) ; i++){
    	*(filename + i) = *(str+4+i);
    }
    filename[strlen(str+4)] = '\0';
    cat(filename);
}

void command_test_alloc(){
    test_alloc();
}

void command_fdt_traverse(){
	fdt_traverse();
}

void command_lab3_basic_1(char *str){
	char * filename;
	for(int i = 0 ; i < strlen(str+5) ; i++){
    	*(filename + i) = *(str+5+i);
    }
    filename[strlen(str+5)] = '\0';
	lab3_basic_1(filename);
}

void command_settimeout(char *str){
	char *message = strchr(str, ' ') + 1;
	char *end_message = strchr(message, ' ');
	*end_message = '\0';
	char *second = end_message + 1;
	add_timer(uart_send_string, atoi(second) ,message);
}

void command_test_buddy_print(){
	test_buddy_print();
}

void command_test_dynamic_alloc_print(){
	test_dynamic_print();
}

