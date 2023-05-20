#ifndef _TIMER_H
#define _TIMER_H

#include "list.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040
typedef struct timer_event{
	
	struct list_head listhead;
	
	unsigned long long interrupt_time; 
	
	void *callback;
	
	char *args; // string message
	
} timer_event_t;

void core_timer_enable(void);

void core_timer_handler(void);

void add_timer(void *callback, unsigned long long after, char *agrs);
unsigned long long get_tick_second(unsigned long long second);
#endif
