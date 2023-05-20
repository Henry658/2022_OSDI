#include "syscall.h"
#include "exception.h"
#include "peripherals/mailbox.h"
#include "mailbox.h"
#include "sched.h"
#include "entry.h"
#include "mm.h"

extern task_struct_t task[TASK_POOL_SIZE];
extern list_head_t zombie_queue_list;

void exec_user(void* addr){
_exec_user(addr);
}

void sys_getpid(trapframe_t * trapframe){
	//lock();
	task_struct_t * current_c = get_current_task();
	task_struct_t * current = (long unsigned int)current_c | KERNEL_VIRTURE_BASE;
	uint64_t task_tid = current->tid;
	trapframe->x[0] = task_tid;
	//unlock();
}

void sys_uart_read(trapframe_t * trapframe){
	//lock();
	char* buf = (char*) trapframe->x[0];
	uint32_t size = trapframe->x[1];
	for(int i =0; i < size ; i++){
		buf[i] = uart_recv();
	}
	buf[size] = '\0';
	trapframe->x[0] = size;
	//unlock();
}

void sys_uart_write(trapframe_t * trapframe){
	//lock();
	int i=0;
	const char* buf = (char*) trapframe->x[0];
	uint32_t size = trapframe->x[1];
	for(i = 0; i < size ; i++){
		uart_send(buf[i]);
	}
	trapframe->x[0] = size;
	//unlock();
}

void sys_exec(trapframe_t * trapframe){
	const char * name = (const char *)trapframe->x[0];
	char * const argv = (char * const)trapframe->x[1];
	trapframe->x[0] = 0;
	do_exec(name);
	/*char * newdata = (unsigned long)get_usr_program_address(name);
	task_struct_t * current = get_current_task();
	current->filesize = get_usr_program_size(name);
	for(int i = 0; i < current->filesize ; i++){
		*(current->data+i) = *(newdata+i);
	}
	trapframe->elr_el1 = (uint64_t)current->data;
	trapframe->sp_el0 = (uint64_t)current->user_sp;
	*/
	
}
void sys_fork(trapframe_t * trapframe){
	
	lock();
	task_struct_t * parent_c = get_current_task();
	task_struct_t * parent = (long unsigned int)parent_c | KERNEL_VIRTURE_BASE;
	
	int child_id = thread_create(ret_from_fork);
	
	task_struct_t * child_c = &task[child_id];
	task_struct_t * child = (long unsigned int)child_c | KERNEL_VIRTURE_BASE;
	
	char * child_kstack = child->kstack_alloc + (PAGE_SIZE - 16);
	char * parent_kstack = parent->kstack_alloc + (PAGE_SIZE - 16);
	
	uint64_t kstack_offset = parent_kstack - (char*)trapframe;
	
	for(uint64_t i = 0 ; i < kstack_offset ; i++){
		*(child_kstack - i) = *(parent_kstack - i);
	}
	
	child->cpu_context.sp = (uint64_t)child_kstack - kstack_offset;
	
	// copy all user pages
	fork_pgd(parent,child);	
	
	trapframe_t * child_trapframe = (trapframe_t *)child->cpu_context.sp;
	child_trapframe->sp_el0 = trapframe->sp_el0;
	
	child_trapframe->x[0] = 0;
	trapframe->x[0] = child_id;
	uart_printf("done!!!\n");
	unlock();

}

void sys_exit(trapframe_t * trapframe){
	do_exit();
}

void sys_mbox_call(trapframe_t * trapframe){
	lock();
	//uart_printf("sys_mbox_call\n");
	unsigned char ch = (unsigned char)trapframe->x[0];
	unsigned int * mbox_c = (unsigned int *)trapframe->x[1];
	
	task_struct_t * current_c = get_current_task();
	task_struct_t * current = (long unsigned int)current_c | KERNEL_VIRTURE_BASE;
	
	mbox_c =(unsigned int *) walk(current->mm.pgd, (uint64_t)mbox_c);
	unsigned int * mbox = (long unsigned int)mbox_c | KERNEL_VIRTURE_BASE;
	
	unsigned int r = (unsigned int)((((unsigned long)mbox) & (~0xF)) | (ch & 0xF));
    
    //wait until full flag unset
    while(*MAILBOX_STATUS & MAILBOX_FULL){
        ; //do nothing
    }
    
    // write address of message + channel to mailbox
    *MAILBOX_WRITE = r;
    
    // wait until response
    while(1){
        // wait until empty flag unset
        while(*MAILBOX_STATUS & MAILBOX_EMPTY){
            ; //do nothing
        }
        if(r == *MAILBOX_READ){
            trapframe->x[0] = (mbox[1] == MAILBOX_CODE_BUF_RES_SUCC);
            unlock();
            return mbox[1] == MAILBOX_CODE_BUF_RES_SUCC;
        }
    }
    trapframe->x[0] = 0;
    unlock();
    return 0;
	
}

void sys_kill(trapframe_t * trapframe){
	lock();
	int pid = trapframe->x[0];
	if(!(pid >= 0 && pid < TASK_POOL_SIZE) || (task[pid].state != RUNNING)){
		//nothing
		uart_printf("kill process not exist, error\n");
		//unlock();
		return;
	}
	else{
		task_struct_t * zombie_c = &task[pid];
		task_struct_t * zombie = (long unsigned int)zombie_c | KERNEL_VIRTURE_BASE;
		
		zombie->state = ZOMBIE;
		list_del_entry( (struct list_head *) zombie);
		list_add_tail(&zombie->head, &zombie_queue_list);
		
	}
	unlock();
	schedule();
}


