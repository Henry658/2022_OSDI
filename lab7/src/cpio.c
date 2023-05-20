#include "mini_uart.h"
#include "tool.h"
#include "string.h"
#include "devicetree.h"
#include "exception.h"
#include "test.h"
#include "cpio.h"
#include "mmu.h"

char str[256];



char * base_address;
void init_cpio(){
	base_address = (KERNEL_VIRTURE_BASE | (long unsigned int)initramfs_callback());
	//base_address = (KERNEL_VIRTURE_BASE | 0x8000000);
}

unsigned int get_cpio_address(){
	return (int)base_address;
}


void ls(){
	uart_printf("%x\n",base_address);
    char * address = (char *)base_address;
    while(!memcmp(address,"070701",6) && memcmp(address+sizeof(cpio_header_type),"TRAILER!!",9)){
        cpio_header_type * cpio_header = (cpio_header_type *)address;
        int namesize = hex2dec(cpio_header->c_namesize,8);
        int filesize = hex2dec(cpio_header->c_filesize,8);
        
        for(int i = 0; i < namesize ; i++){
        	str[i] = *(address + sizeof(cpio_header_type) + i);
        }
        str[namesize] = '\0';
        
	uart_printf("%s size: %d \n", str, filesize);   

        if ((sizeof(cpio_header_type) + namesize) % 4){
            address += (((sizeof(cpio_header_type)+namesize) / 4) +1 ) * 4;
        }
        else{
            address += sizeof(cpio_header_type) + namesize;
        }
        
        if ((filesize)%4){
            address += ((filesize / 4 ) +1) * 4;
        }
        else{
            address += filesize;
        }
    }
}


void cat(char * str){
	char * address = (char *)base_address;
    char * filename;
    int found = 0;
    while(!memcmp(address,"070701",6) && memcmp(address+sizeof(cpio_header_type),"TRAILER!!",9)){
        cpio_header_type * cpio_header = (cpio_header_type *)address;
        int namesize = hex2dec(cpio_header->c_namesize,8);
        int filesize = hex2dec(cpio_header->c_filesize,8);
        filename = address + sizeof(cpio_header_type);
        if(!memcmp(str,filename,strlen(str)+1)){
        
            uart_puts_width(address + sizeof(cpio_header_type)+namesize,filesize);
           // uart_send_string("\r\n");
            found = 1;
            break;
        }
        if ((sizeof(cpio_header_type) + namesize) % 4){
            address += (((sizeof(cpio_header_type)+namesize) / 4) +1 ) * 4;
        }
        else{
            address += sizeof(cpio_header_type) + namesize;
        }
        
        if ((filesize)%4){
            address += ((filesize / 4 ) +1) * 4;
        }
        else{
            address += filesize;
        }
    }
    if(!found){
        uart_send_string("file not found!\r\n");
    }
    filename = NULL;
}

int get_usr_program_address(char * str){
	char * address = (char *)base_address;
    char * filename;
    int found = 0;
    char * file;
    while(!memcmp(address,"070701",6) && memcmp(address+sizeof(cpio_header_type),"TRAILER!!",9)){
        cpio_header_type * cpio_header = (cpio_header_type *)address;
        int namesize = hex2dec(cpio_header->c_namesize,8);
        int filesize = hex2dec(cpio_header->c_filesize,8);
        filename = address + sizeof(cpio_header_type);
        if(!memcmp(str,filename,strlen(str)+1)){
           // uart_send_string("\r\n");
            found = 1;
            unsigned int offset = sizeof(cpio_header_type) + namesize;
            offset = offset%4==0 ? offset : (offset+4-offset%4);
            return address + offset ;
            //return filesize;
            break;
        }
        if ((sizeof(cpio_header_type) + namesize) % 4){
            address += (((sizeof(cpio_header_type)+namesize) / 4) +1 ) * 4;
        }
        else{
            address += sizeof(cpio_header_type) + namesize;
        }
        
        if ((filesize)%4){
            address += ((filesize / 4 ) +1) * 4;
        }
        else{
            address += filesize;
        }
    }
    if(!found){
        uart_send_string("file not found!\r\n");
    }
    filename = NULL;
}

int get_usr_program_size(char * str){
	char * address = (char *)base_address;
    char * filename;
    int found = 0;
    while(!memcmp(address,"070701",6) && memcmp(address+sizeof(cpio_header_type),"TRAILER!!",9)){
        cpio_header_type * cpio_header = (cpio_header_type *)address;
        int namesize = hex2dec(cpio_header->c_namesize,8);
        int filesize = hex2dec(cpio_header->c_filesize,8);
        filename = address + sizeof(cpio_header_type);
        if(!memcmp(str,filename,strlen(str)+1)){
           // uart_send_string("\r\n");
            found = 1;
            return filesize;
            break;
        }
        if ((sizeof(cpio_header_type) + namesize) % 4){
            address += (((sizeof(cpio_header_type)+namesize) / 4) +1 ) * 4;
        }
        else{
            address += sizeof(cpio_header_type) + namesize;
        }
        
        if ((filesize)%4){
            address += ((filesize / 4 ) +1) * 4;
        }
        else{
            address += filesize;
        }
    }
    if(!found){
        uart_send_string("file not found!\r\n");
    }
    filename = NULL;
}

void lab3_basic_1(char *str){
	void * user_program = get_usr_program_address(str);
	uart_printf("%x\n",user_program);
	exec_user(user_program);
}
