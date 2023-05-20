#ifndef _ALLOC_H
#define _ALLOC_H

#include "list.h"
#include "buddy.h"

#define NULL ((void*)-1)

void * simple_alloc(unsigned int size);

// dynamic_alloc 

#define CHUNK_SIZE 16 // base chunk size, unit byte

typedef struct free_chunk_struct{
	struct list_head head;
	void * address;
	void * next;
	int mul;
	int is_used;
}free_chunk_t;

void free_chunk_init();

void test_dynamic_print();

void * dynamic_alloc(unsigned int size);

void dynamic_free(void * ptr);

free_chunk_t * bestfit(unsigned int size);

void CHUNK_get_page_and_split();

void print_free_chunk_list();

void print_chunk_used_list();

free_chunk_t * get_page_and_remove_in_chunk_used_list(free_chunk_t ** head, void * ptr);

int free_chunk_list_size();
#endif
