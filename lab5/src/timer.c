// reference from 13570and2468
// nice code

#include "timer.h"
#include "alloc.h"
#include "sysreg.h"
#include "mini_uart.h"
#include "sched.h"
#include "typedef.h"
#include "string.h"

struct list_head *timer_event_list;

int start = 0;

void timer_list_init(){
	uint64_t tmp;
	asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
	tmp |= 1;
	asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
	INIT_LIST_HEAD(timer_event_list);
}

void core_timer_init_enable(){
	__asm__ __volatile__(
	
	"mov x1, 1 \n\t"
	"msr cntp_ctl_el0, x1 \n\t" // enable
	"mrs x1, cntfrq_el0 \n\t"
	"mov x2, 0x1 \n\t"
	"mul x1, x1, x2 \n\t"
	"msr cntp_tval_el0, x1 \n\t"
	
	"mov x2, 2 \n\t"
	"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL)"\n\t" 
	"str w2, [x1]\n\t" // unmask timer interrupt
	);
	
}

void core_timer_enable(){
	__asm__ __volatile__(
	
	"mov x1, 1 \n\t"
	"msr cntp_ctl_el0, x1 \n\t" // enable

	"mov x2, 2 \n\t"
	"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL)"\n\t" 
	"str w2, [x1]\n\t" // unmask timer interrupt
	);
	
}

void core_timer_disable(){
	__asm__ __volatile__(
	"mov x0, 0  \n\t"
	"ldr x1, ="XSTR(CORE0_TIMER_IRQ_CTRL)"\n\t" 
	"str w0, [x1]\n\t" // unmask timer interrupt
	);
}

void core_timer_handler(){
	disable_interrupt();
	if( list_empty(timer_event_list)){
		//uart_printf("timer di di da\n");
		unsigned long long cntfrq_el0;
		__asm__ __volatile__("mrs %0, cntfrq_el0": "=r"(cntfrq_el0)); //tick frequency
		register unsigned int expired_time = (cntfrq_el0 >> 5);
		__asm__ __volatile__("msr cntp_tval_el0, %0": :"r"(expired_time)); //tick frequency
		task_struct_t * current = get_current_task();
		if(--current->counter <= 0){
			current->counter = 0;
			current->need_resched = 1;
		}
		enable_interrupt();
		return;
		 
	}
	// the most smallest interrupt value is next to the timer_event_list (header)
	timer_event_callback((timer_event_t *)timer_event_list->next); //do callback and set new interrupt 
	enable_interrupt();
}

void timer_event_callback(timer_event_t * timer_event){
	uart_printf("\nnumber of timer list: %d\n",timer_list_size());
	list_del_entry( (struct list_head *) timer_event);
	((void (*)(char *))timer_event->callback)(timer_event->args);
	
	free(timer_event->args); //todo
	free(timer_event); //todo
	uart_printf("\nnumber of timer list: %d\n",timer_list_size());
	if(!list_empty(timer_event_list)){
		//set the interrupt to the most smallest one in the list
		set_core_timer_interrupt_by_tick(((timer_event_t *)timer_event_list->next)->interrupt_time);	
	}
	else{
		set_core_timer_interrupt(1); //disable timer interrupt (set a very big value)
	}

}


void add_timer(void *callback, unsigned long long after, char * args){
	timer_event_t * this_timer_event = simple_alloc(sizeof(timer_event_t)); // node of the event_list
	
	this_timer_event->args = simple_alloc(strlen(args) + 1);
	strcpy(this_timer_event->args, args);
	this_timer_event->interrupt_time = get_tick_second(after); //store interrupt time
	this_timer_event->callback = callback; // store callback function
	
	INIT_LIST_HEAD(&this_timer_event->listhead); // init list head
	
	struct list_head *pointer; // a current list_head pointer
	
	list_for_each(pointer, timer_event_list){//traverse list
		if( ((timer_event_t *)pointer)->interrupt_time > this_timer_event->interrupt_time ){
			list_add(&this_timer_event->listhead, pointer->prev); //add node in the list (sort)
			break;
		}
	
	}
	// if the new event is the biggest one, then we add it to tail, prev to the header
	if(list_is_head(pointer, timer_event_list)){
		list_add_tail(&this_timer_event->listhead, timer_event_list);
	}
	// set the time interrupt for the first evnet
	// (the most smallest interrupt time in the list)
	set_core_timer_interrupt_by_tick(((timer_event_t *)timer_event_list->next)->interrupt_time);
	
}

unsigned long long get_tick_second(unsigned long long second){
	unsigned long long cntpct_el0=0;
	unsigned long long cntfrq_el0=0;
	__asm__ __volatile__("mrs %0, cntpct_el0 \n\t" : "=r" (cntpct_el0));
	__asm__ __volatile__("mrs %0, cntfrq_el0 \n\t" : "=r" (cntfrq_el0));
	return (cntpct_el0 + cntfrq_el0 * second);
}

// set timer interrupt time to [expired_time] second after now (relatively)
void set_core_timer_interrupt(unsigned long long expired_time){
	__asm__ __volatile__(
		"mrs x1, cntfrq_el0 \n\t"
		"mul x1, x1, %0 \n\t"
		"msr cntp_tval_el0, x1 \n\t"
		 : "=r" (expired_time));
}

// set the timer interrupt by tick (directly) (absolutely)
void set_core_timer_interrupt_by_tick(unsigned long long tick){
	__asm__ __volatile__("msr cntp_cval_el0, %0 \n\t" : "=r" (tick));
}

int timer_list_size(){
	int count = 0;
	struct list_head *pointer; // a current list_head pointer
	list_for_each(pointer, timer_event_list){//traverse list
		count++;
	}
	return count;
}
