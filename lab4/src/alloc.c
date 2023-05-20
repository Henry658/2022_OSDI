#include "alloc.h"

extern char _alloc_start;
static char * top = & _alloc_start;

void * simple_alloc(unsigned int size){
	char *r = top+0x10;
	if(size<0x18)size=0x18;
	size = size + 0x7;
	size = 0x10 + size - size % 0x10;
	*(unsigned int*)(r-0x8) = size;
	top += size;
	return r;
}



/*extern int __kernel_size;

void * freeptr = 0x80000 + &__kernel_size;

void * simple_alloc(unsigned int size){
    freeptr += size;
    return freeptr - size;

}*/
//Todo


void free(void * ptr){
;
}


// dynamic_alloc 
#define CHUNK_RANGE 10

struct list_head * free_chunk_list;


int chunk_distribution[CHUNK_RANGE] = {15, 15, 7, 5, 5, 5, 5, 5, 5, 5, 5};

free_chunk_t * chunk_used_list = NULL;

void free_chunk_init(){
	INIT_LIST_HEAD(free_chunk_list);
}

void test_dynamic_print(){
	print_free_chunk_list();
	uart_printf("num of free chunk list: %d\n",free_chunk_list_size());
	void * ptr_1 = dynamic_alloc(13);
	void * ptr_2 = dynamic_alloc(34);
	void * ptr_3 = dynamic_alloc(23);
	void * ptr_4 = dynamic_alloc(147);
	print_free_chunk_list();	
		
	print_chunk_used_list();
	uart_printf("num of free chunk list: %d\n",free_chunk_list_size());
	
	uart_printf("chunk physical address: %x\n",ptr_4);
	uart_printf("chunk physical address: %x\n",ptr_3);
	uart_printf("chunk physical address: %x\n",ptr_2);
	uart_printf("chunk physical address: %x\n",ptr_1);
	
	uart_printf("\n");

	//dynamic_free(ptr_4);
	//dynamic_free(ptr_3);
	//dynamic_free(ptr_2);
	//dynamic_free(ptr_1);
	print_free_chunk_list();	
	print_chunk_used_list();
	uart_printf("num of free chunk list: %d\n",free_chunk_list_size());
}

void * dynamic_alloc(unsigned int size){ // only work at size under 160
	void * ptr;
	free_chunk_t * chunk;
	if(list_empty(free_chunk_list)){// get the new page to the free chunk pool
		CHUNK_get_page_and_split();
	}
	// find the proper chunk use the bestfit strategy
	chunk = bestfit(size);
	chunk->is_used = 1;
	ptr = chunk->address;
		
	// add to used list
	chunk->next = chunk_used_list;
	chunk_used_list = chunk;
	return ptr;
}

void dynamic_free(void * ptr){
	free_chunk_t * chunk = get_page_and_remove_in_chunk_used_list(&chunk_used_list, ptr);
	chunk->is_used = 0;
	list_add_tail(&chunk->head, free_chunk_list);
}


free_chunk_t * bestfit(unsigned int size){
	int min=1215752191;	
	int find = 0;
	int c = 0;
	struct list_head *best_pointer;
	struct list_head *pointer; // a current list_head pointer
	
	list_for_each(pointer, free_chunk_list){//traverse list
		
		int chunk_size = ((free_chunk_t *)pointer)->mul * CHUNK_SIZE;

		if(chunk_size >= size){
			find = 1;
			if((chunk_size - size) < min){
				min = (chunk_size - size);
				best_pointer = pointer;
			}
		}
	}
	
	if(!find){
		CHUNK_get_page_and_split();
		list_for_each(pointer, free_chunk_list){//traverse list
		int chunk_size = ((free_chunk_t *)pointer)->mul * CHUNK_SIZE;
		if(chunk_size >= size){
			find = 1;
			if((chunk_size - size) < min){
				min = (chunk_size - size);
				best_pointer = pointer;
			}
		}
	}
	}
	
	list_del_entry(best_pointer);
	return (free_chunk_t *)best_pointer;
}

void CHUNK_get_page_and_split(){
	int i,j;
	void * chunk_base = alloc_page(4096);
	void * cur_address = chunk_base; 
	
	for(i = 0; i < CHUNK_RANGE; ++i){
		for(j = 0; j < chunk_distribution[i]; ++j){
			free_chunk_t * chunk = simple_alloc(sizeof(free_chunk_t));
			chunk->address = cur_address;
			chunk->next = NULL;
			chunk->mul = (i+1);
			chunk->is_used = 0;
			list_add_tail(&chunk->head, free_chunk_list);
			cur_address += (chunk->mul * CHUNK_SIZE); 
		}
	}
}

void print_free_chunk_list(){
	int i,count = 0;
	struct list_head *pointer;
	int temp_chunk_distribution[CHUNK_RANGE] = {0};
	
	uart_printf("free chunk list: %d\n",free_chunk_list_size());
	list_for_each(pointer, free_chunk_list){//traverse list
		temp_chunk_distribution[(((free_chunk_t *)pointer)->mul - 1)]++;
	}
	for(i = 0; i < CHUNK_RANGE ; ++i ){
		uart_printf("free chunk size: %d have %d\n", (i+1)*CHUNK_SIZE, temp_chunk_distribution[i]);	
	}
}

void print_chunk_used_list(){
	free_chunk_t * cur = chunk_used_list;
	uart_printf("chunk used list: \n");
	while(cur!=NULL){
		uart_printf("chunk address: %x chunk mul: %d chunk is_used: %d\n", cur->address, cur->mul, cur->is_used);
		cur = cur->next;
	}
}

free_chunk_t * get_page_and_remove_in_chunk_used_list(free_chunk_t ** head, void * ptr){
	
	free_chunk_t *temp = *head, *prev;
	if(temp !=NULL && (ptr == temp->address)){
		*head = temp->next;
		return temp; 
	}
	while(temp !=NULL && (ptr != (temp->address))){
		prev = temp;
		temp = temp->next;
	}
	if(temp == NULL){
		uart_printf("element is not in the chunk used list\n");
		return NULL;
	}
	
	prev->next = temp->next;
	return temp;
}

int free_chunk_list_size(){
	int count = 0;
	struct list_head *pointer; // a current list_head pointer
	list_for_each(pointer, free_chunk_list){//traverse list
		count++;
	}
	return count;
}
