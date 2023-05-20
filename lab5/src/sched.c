#include "sched.h"
#include "buddy.h"
#include "entry.h"
#include "sysreg.h"
#include "mini_uart.h"

task_struct_t task[TASK_POOL_SIZE];
list_head_t run_queue_list;
list_head_t zombie_queue_list;

long tid_count = 1000;

void scheduler_init(){
	task_init();
	run_queue_init();
	core_timer_init_enable();
	thread_create(idle);
}

void task_init(){
	for(int i = 0; i < TASK_POOL_SIZE ; i++){
		task[i].tid = i;
		task[i].state = EXIT;
		task[i].need_resched = 0;
		INIT_LIST_HEAD(&task[i].head);
		task[i].ustack_alloc = NULL;
		task[i].kstack_alloc = NULL;
		task[i].user_sp = NULL;
	}
	// idle task do nothing

	task[0].state = RUNNING;
	task[0].counter = TASKEPOCH;
	
	//current = &task[0];
	set_current_task(&task[0]);
}

void run_queue_init(){
	//init run_queue_list
	INIT_LIST_HEAD(&run_queue_list);
	for(int i = 0; i < TASK_POOL_SIZE ; i++){
		if(task[i].state == RUNNING){
			list_add_tail(&task[i].head, &run_queue_list);
		}	
	}
	//init run_queue_list
	INIT_LIST_HEAD(&zombie_queue_list);
	
}

void idle(){
	while(1){
		//uart_printf("thread : 1\n");
		//disable_interrupt();
		kill_zombie();
		//enable_interrupt();
		schedule();
	}
}

int thread_create(unsigned long fn){// kernel thread
	lock();
	task_struct_t *new_task;
	for(int i = 0 ; i < TASK_POOL_SIZE ; i++){
		if(task[i].state == EXIT){
			new_task = &task[i];
			break; 
		}
	}

	new_task->state = RUNNING;
	new_task->counter = TASKEPOCH;
	new_task->need_resched = 0;
	
	new_task->kstack_alloc = (unsigned long) alloc_page(PAGE_SIZE);
	new_task->ustack_alloc = (unsigned long) alloc_page(PAGE_SIZE);
	
	uart_printf("kernel stack %x\n", new_task->ustack_alloc);
	uart_printf("user stack %x\n", new_task->kstack_alloc);
	
	new_task->cpu_context.lr = (unsigned long)fn;
	new_task->cpu_context.fp = new_task->kstack_alloc + (PAGE_SIZE - 16);
	new_task->cpu_context.sp = new_task->kstack_alloc + (PAGE_SIZE - 16);
	new_task->user_fp = new_task->ustack_alloc + (PAGE_SIZE - 16);
	new_task->user_sp = new_task->ustack_alloc + (PAGE_SIZE - 16);
	uart_printf("new_task tid: %d\n", new_task->tid);
	list_add_tail(&new_task->head, &run_queue_list);
	unlock();
	return new_task->tid;
}

void exec_thread(){
	char * str = "syscall.img";
	task_struct_t * current = get_current_task();
	do_exec(str);
}


void schedule(){
	lock();
	task_struct_t * current = get_current_task();
        current = (task_struct_t *) current->head.next;
    	if(list_is_head(&current->head, &run_queue_list)){
    		current = (task_struct_t *) current->head.next;
    	}
    	//uart_printf("tid: %d\n", current->tid);
	//set_current_task(current);
	context_switch(current);
	unlock();
}

void context_switch(task_struct_t * next){
	
	//core_timer_disable();
	task_struct_t * prev = get_current_task();
	if(prev->state == RUNNING){
		//list_add_tail(&prev->head, &run_queue_list);
	}
	set_current_task(next);
	//enable_interrupt();
	cpu_switch_to(&prev->cpu_context, &next->cpu_context);
}

void do_exec(char *func){
	task_struct_t * current = get_current_task();
	void * user_program = get_usr_program_address(func);
	int filesize = get_usr_program_size(func);
	
	current->data = alloc_page(filesize);
	current->filesize = filesize;
	//current->cpu_context.lr = (uint64_t)current->data;
	char* data = (char *)user_program;
	//copy file into thread's data
	uart_printf("fs %d\n", filesize);
	for(int i = 0; i < filesize ; i++){
		*(current->data+i) = *(data+i);
	}
	uart_printf("current pid %d...\n", current->tid);
	asm volatile("msr sp_el0, %0" : : "r"(current->ustack_alloc+PAGE_SIZE-16));
	asm volatile("msr elr_el1, %0" : : "r"(current->data));
	asm volatile("msr spsr_el1, xzr");
	asm volatile("eret");
}


void do_exit(){
	lock();
	task_struct_t * current = get_current_task();
	uart_printf("exit tid: %d\n", current->tid);
	current->state = ZOMBIE;
	task_struct_t * zombie = current;
	list_del_entry( (struct list_head *) zombie);
	list_add_tail(&zombie->head, &zombie_queue_list);
	unlock();
	schedule();
}

void run_queue_list_traversal(){
	struct list_head *pointer;
	list_for_each(pointer, &run_queue_list){//traverse list
		uart_printf("run queue thread tid: %d\n", ((task_struct_t *)pointer)->tid);
	}
	uart_printf("\n");

}

int run_queue_list_size(){
	int count = 0;
	struct list_head *pointer; // a current list_head pointer
	list_for_each(pointer, &run_queue_list){//traverse list
		count++;
	}
	return count;
}

void kill_zombie(){
	if(list_empty(&zombie_queue_list)){
		return;
	}
	else{
		task_struct_t * task;
		struct list_head *pointer;
		list_for_each(pointer, &zombie_queue_list){//traverse list
			task = (task_struct_t *) pointer;
			uart_printf("kernel stack %x\n", task->kstack_alloc);
			uart_printf("user stack %x\n", task->ustack_alloc);
			free_page(task->kstack_alloc);
			free_page(task->ustack_alloc);
			task->state = EXIT;
			list_del_entry( (struct list_head *) task);
		}
	
	}
}

void zombie_queue_list_traversal(){
	struct list_head *pointer;
	list_for_each(pointer, &zombie_queue_list){//traverse list
		uart_printf("thread tid: %d\n", ((task_struct_t *)pointer)->tid);
	}

}

int zombie_queue_list_size(){
	int count = 0;
	struct list_head *pointer; // a current list_head pointer
	list_for_each(pointer, &zombie_queue_list){//traverse list
		count++;
	}
	return count;
}

void syscall(){
	char * str = "syscall.img";
	exec_thread(str);
}
