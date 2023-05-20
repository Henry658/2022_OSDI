#ifndef _BUDDY_H
#define _BUDDY_H

#include "list.h"

#define NULL ((void*)-1)

typedef struct free_area_struct{
	struct list_head free_page_list;
	int order;
	int num_free_page;
}free_area_t;

typedef struct free_page_struct{
	struct list_head head;
	void * next;
	int index;
	int order;
	int is_used; // 1=allocated, 0=free, -1=system_reserve
}free_page_t;

typedef struct frame_array_struct{
	free_page_t * next;
	//int value;
}frame_array_t;

void buddy_init();

void print_buddy_info_log();

void test_buddy_print();

void page_split(int order,free_page_t * page);

void page_coalesce(int order, free_page_t * page_left, free_page_t * page_right);

void * alloc_page(unsigned int size);

void free_page(void * ptr);

void recursive_find_buddy(int order);

void recursive_coalesce_buddy(free_page_t * page);

void print_used_list();

free_page_t * get_page_and_in_used_list(void * ptr);

void enroll(free_page_t * page);

void un_enroll(free_page_t * page);

#endif
