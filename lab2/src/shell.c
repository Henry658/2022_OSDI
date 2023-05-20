#include "mini_uart.h"
#include "command.h"
#include "string.h"

#define MAX_STR_BUFFER 128

enum shell_status{
    Read,
    Parse
};

void shell_standby(){
    char str[MAX_STR_BUFFER];
    enum shell_status status = Read;
    while(1){
        switch(status){
            case Read:
                shell_input(str);
                status = Parse;
                break;
            case Parse:
                shell_command(str);
                status = Read;
                break;
        }
    }
}



void shell_input(char *str){
    int idx = 0, end = 0, i;
    char c;
    str[0] = '\0';
    while((c = uart_recv()) != '\r'){
    	//uart_send(c);
        if( c == 8 || c == 127){
            if(idx > 0){
                idx--;
                for(i = idx; i < end; i++){
                    str[i] = str[i+1];
                }
                str[--end] = '\0';
            }
        }
        else{
            if(idx < end){
                for(i = end; i > idx; i--){
                    str[i] = str[i-1];
                }
            }
            str[idx++] = c;
            str[++end] = '\0';
            
        }
        uart_send(c);
    }
    uart_send_string("\r\n");

}

void shell_command(char* str){
    if(!strcmp(str,"")){
        return;
    }
    else if(!strcmp(str,"help")){
        command_help();
    }
    else if(!strcmp(str,"hello")){
        command_hello();
    }
    else if(!strcmp(str,"reboot")){
        command_reboot();
    }
    else if(!strcmp(str,"cancel reboot")){
        command_cancel_reboot();
    }
    else if(!strcmp(str,"ls")){
        command_ls();
    }
    else if(!strcmp_len(str,"cat",3)){
        command_cat(str);
    }
    else if(!strcmp(str,"test alloc")){
        command_test_alloc();
    }
    else if(!strcmp(str,"fdt_traverse")){
    	command_fdt_traverse();
    }
    else{
        command_not_found();
    }
}
