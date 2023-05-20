#ifndef _TEST_T
#define _TEST_T
#include "list.h"
typedef struct test_struct{
	struct list_head head;
	int data;
}test_struct_t;

void test_alloc();
void test();
void get_el_value();
#endif
