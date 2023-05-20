#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "utils.h"
#include "sched.h"
#include "vfs.h"

void test_1(){
	file_t* a = vfs_open("hello", 0);
	//assert(a == NULL);
	uart_printf("addr: %x\n", a);
	a = vfs_open("hello", O_CREAT);
	uart_printf("addr: %x\n", a);
	//assert(a != NULL);
	vfs_close(a);
	//uart_printf("f\n");
	
	file_t* b = vfs_open("hello", 0);
	uart_printf("addr: %x\n", b);
	b = vfs_open("hello", O_CREAT);
	uart_printf("addr: %x\n", b);
	//while(1);
	
	//assert(b != NULL);
	vfs_close(b);
	uart_printf("done\n");
	do_exit();
}

void test_2(){
    struct file* a = vfs_open("hello", O_CREAT);
    struct file* b = vfs_open("world", O_CREAT);
    vfs_write(a, "Hello ", 6);
    vfs_write(b, "World!", 6);
    vfs_close(a);
    vfs_close(b);

    char buf[512];
    b = vfs_open("hello", 0);
    a = vfs_open("world", 0);
    int sz;
    sz = vfs_read(b, buf, 100);
    sz += vfs_read(a, buf + sz, 100);
    buf[sz] = '\0';
    uart_printf("%s\n", buf); // should be Hello World!
    uart_printf("done\n");
  
    do_exit();

}

void kernel_main(void){

	system_init();
	delay(10000000);
	uart_printf("# Hello I'm raspi3 B+ model\n");
    
   	mailbox_board_revision();
  	mailbox_vc_memory();
	int el = get_el();
	uart_printf("Exception level %d \n",el);
	
	scheduler_init();
	
	uart_printf("thread create done\n");
	
	uart_printf("run queue list size: %d\n", run_queue_list_size());
	uart_printf("zombie queue list size: %d\n", zombie_queue_list_size());
	
	thread_create(exec_thread);
	//while(1){
		//shell_standby();
	//}
	//thread_create(test_2);
	
	while(1){
		;//schedule();
	}
}
