#include "mini_uart.h"
#include "reboot.h"
#include "loader.h"
 
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
    uart_send_string("\r\n");
}

void command_hello(){
    uart_send_string("Hello I'm bootloader\r\n");
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

void command_loading_kernel(){
    loading();
}

