#include "alloc.h"
#include "mini_uart.h"
void test_alloc(){
    unsigned int size = 5;
    char * ptr = simple_alloc(4*size);
    for(int i = 0; i < size ; i ++){
       *(ptr + i) = (char)(65+i);
    }
    uart_send_string(ptr);
}
