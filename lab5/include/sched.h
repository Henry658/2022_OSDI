#ifndef _SCHED_H
#define _SCHED_H

#define PAGE_SIZE 4096
#define TASK_POOL_SIZE 64
#define TASKEPOCH 1

#include "list.h"
#include "typedef.h"

typedef struct cpu_context{
	// ARM calling convention
	// x0 ~ x18 can be overwritten by the called function
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t fp; // x29
	uint64_t lr; // x30
	uint64_t sp;
}cpu_context_t;

enum task_state{
	RUNNING,
	ZOMBIE,
	EXIT,
};

typedef struct task_struct{
	struct list_head head;
	cpu_context_t cpu_context;
	uint64_t tid;
	uint64_t counter;
	uint64_t need_resched;
	enum task_state state;
	uint64_t ustack_alloc;
	uint64_t kstack_alloc;
	uint64_t user_fp;
	uint64_t user_sp;
	uint64_t filesize;
	char * data;
}task_struct_t;

//task_struct_t * current;

void scheduler_init();

void task_init();

void run_queue_init();

void idle();

int thread_create(unsigned long fn);

void exec_thread();

void schedule();

void context_switch(task_struct_t * next);

void do_exec(char * func);

void do_exit();

void run_queue_list_traversal();

int run_queue_list_size();

void kill_zombie();

void zombie_queue_list_traversal();

int zombie_queue_list_size();

void syscall();
#endif
