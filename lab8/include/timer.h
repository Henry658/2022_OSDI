#ifndef _TIMER_H
#define _TIMER_H

#include "list.h"

#define KERNEL_VIRTURE_BASE       0xFFFF000000000000
#define CORE0_TIMER_IRQ_CTRL 0x40000040 + KERNEL_VIRTURE_BASE
typedef struct timer_event{
	
	struct list_head listhead;
	
	unsigned long long interrupt_time; 
	
	void *callback;
	
	char *args; // string message
	
} timer_event_t;

void timer_list_init();

void core_timer_enable();

void core_timer_disable();

void core_timer_handler();

void timer_event_callback(timer_event_t * timer_event);


void add_timer(void *callback, unsigned long long after, char * args);

unsigned long long get_tick_second(unsigned long long second);

// set timer interrupt time to [expired_time] second after now (relatively)
void set_core_timer_interrupt(unsigned long long expired_time);

// set the timer interrupt by tick (directly) (absolutely)
void set_core_timer_interrupt_by_tick(unsigned long long tick);

int timer_list_size();
#endif
