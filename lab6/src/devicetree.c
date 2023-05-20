#include "mini_uart.h"
#include "string.h"
#include "mmu.h"

typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009
 

typedef struct{
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_string;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
}fdt_header_type;

typedef struct{
    uint64_t adress;
    uint64_t size;
}fdt_reserve_entry_type;

typedef struct{
    uint32_t len;
    uint32_t nameoff;
}lexical_structure_type;


register char *x11 asm("x11");
char * dtb_base;

void init_dtb(){
	dtb_base = x11;
	dtb_base = (KERNEL_VIRTURE_BASE | (long unsigned int)dtb_base);
}

unsigned int get_dtb_address(){
	return (unsigned int)dtb_base;
}

void print_header_info(char * dtb_base, fdt_header_type * fdt_header){
    uart_send_string("dtb_base: ");
    uart_print_long(dtb_base);
    uart_send_string("\r\n");
    
    uart_send_string("magic: ");
    uart_print_uint(swap_endian_uint32(fdt_header->magic));
    uart_send_string("\r\n");
    
    uart_send_string("totalsize: ");
    uart_print_uint(swap_endian_uint32(fdt_header->totalsize));
    uart_send_string("\r\n");
    
    uart_send_string("off_dt_struct: ");
    uart_print_uint(swap_endian_uint32(fdt_header->off_dt_struct));
    uart_send_string("\r\n");
   
    uart_send_string("off_dt_string: ");
    uart_print_uint(swap_endian_uint32(fdt_header->off_dt_string));
    uart_send_string("\r\n");
  
	uart_send_string("off_mem_rsvmap: ");
    uart_print_uint(swap_endian_uint32(fdt_header->off_mem_rsvmap));
    uart_send_string("\r\n");
    
    uart_send_string("version: ");
    uart_print_uint(swap_endian_uint32(fdt_header->version));
    uart_send_string("\r\n");
    
    uart_send_string("last_comp_version: ");
    uart_print_uint(swap_endian_uint32(fdt_header->last_comp_version));
    uart_send_string("\r\n");
    
    uart_send_string("boot_cpuid_phys: ");
    uart_print_uint(swap_endian_uint32(fdt_header->boot_cpuid_phys));
    uart_send_string("\r\n");
    
    uart_send_string("size_dt_strings: ");
    uart_print_uint(swap_endian_uint32(fdt_header->size_dt_strings));
    uart_send_string("\r\n");
    
    uart_send_string("size_dt_struct: ");
    uart_print_uint(swap_endian_uint32(fdt_header->size_dt_struct));
    uart_send_string("\r\n");

}

void fdt_traverse(){
    char * t_address = (char *)dtb_base;
    fdt_header_type * fdt_header = (fdt_header_type *) dtb_base;
    
	//print_header_info(dtb_base, fdt_header);
	
	char * dt_struct_address = dtb_base + swap_endian_uint32(fdt_header->off_dt_struct) ;
	
	char * dt_string_address = dtb_base + swap_endian_uint32(fdt_header->off_dt_string) ;
	int re = 5, i=0;
	int * begin_node = (int *) dt_struct_address;
	uint32_t cpio_start_address;
		while(swap_endian_uint32(*begin_node++)==FDT_BEGIN_NODE){
			while(swap_endian_uint32(*begin_node++)!=FDT_END_NODE){
				if(swap_endian_uint32(*begin_node++)==FDT_PROP){
					uint32_t len = swap_endian_uint32(*begin_node++);
					uint32_t nameoff = swap_endian_uint32(*begin_node++);
					uart_send_string(dt_string_address + nameoff);
					uart_send_string("\r\n");
					delay(150000);
				}
			}		
		}
}

char * initramfs_callback(){
    char * t_address = (char *)dtb_base;
    fdt_header_type * fdt_header = (fdt_header_type *) dtb_base;
    
	//print_header_info(dtb_base, fdt_header);
	
	char * dt_struct_address = dtb_base + swap_endian_uint32(fdt_header->off_dt_struct) ;
	
	char * dt_string_address = dtb_base + swap_endian_uint32(fdt_header->off_dt_string) ;
	int re = 5, i=0;
	int * begin_node = (int *) dt_struct_address;
	uint32_t cpio_start_address, cpio_end_address;
		while(swap_endian_uint32(*begin_node++)==FDT_BEGIN_NODE){
			while(swap_endian_uint32(*begin_node++)!=FDT_END_NODE){
				if(swap_endian_uint32(*begin_node++)==FDT_PROP){
					uint32_t len = swap_endian_uint32(*begin_node++);
					uint32_t nameoff = swap_endian_uint32(*begin_node++);
					if(!strcmp((dt_string_address + nameoff),"linux,initrd-start")){
						cpio_start_address = swap_endian_uint32(*begin_node);
						//uart_print_uint(swap_endian_uint32(*begin_node++));					
						//uart_send_string("\r\n");
						
					}
					if(!strcmp((dt_string_address + nameoff),"linux,initrd-end")){
						cpio_end_address = swap_endian_uint32(*begin_node);
						//uart_print_uint(swap_endian_uint32(*begin_node++));					
						//uart_send_string("\r\n");
						
					}
				}
			}		
		}
	uart_send_string("# cpio address start at: ");
	uart_print_uint(cpio_start_address);
	uart_send_string(" cpio address end at: ");
	uart_print_uint(cpio_end_address);	
	uart_send_string("\r\n");
	return (char *)cpio_start_address;
}
