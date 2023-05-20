#include "mini_uart.h"
#include "loader.h"
void loading(){

    uart_send_string("Please send kernel via uart!\r\n");

    int image_size = 0, i;
    for(i = 0; i < 4; i++){
        image_size <<= 8;
        image_size |= (int)uart_read_raw();
    }

    int image_checksum = 0;
    for(i = 0; i < 4; i++){
        image_checksum <<= 8;
        image_checksum |= (int)uart_read_raw();
    }

    char * kernel_address = (char *) 0x80000;

    for(i = 0; i < image_size; i++){
        char byte = uart_read_raw();
        *(kernel_address + i) = byte;
        image_checksum -= (int)byte;
    }
    
    if(image_checksum != 0){
    	uart_send_string("kernel load failed!\r\n");
    }
    else{
        uart_send_string("kernel load success!\r\n");
        void (*start_kernel)(void) = (void *)kernel_address;
        start_kernel();
    }

}
