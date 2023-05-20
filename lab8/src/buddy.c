#include "buddy.h"
#include "mini_uart.h"
#include "math.h"
#include "alloc.h"
#include "list.h"
#include "mmu.h"

extern unsigned char kernel_start;
extern unsigned char kernel_end;

#define BUDDY_BASE   0x10000000   //0x00 //0x10000000
#define BUDDY_END    0x1fffffff   //0x3C000000//0x20000000

#define BUDDY_BASE_ADDRESS  (KERNEL_VIRTURE_BASE | BUDDY_BASE)
#define BUDDY_END_ADDRESS   (KERNEL_VIRTURE_BASE | BUDDY_END)

#define PAGE_SIZE 4096 // 4kb

#define PAGE_FRAMES_NUM (BUDDY_END_ADDRESS +1 - BUDDY_BASE_ADDRESS) / PAGE_SIZE
#define MAX_BUDDY_ORDER 11 //(LOG2(PAGE_FRAMES_NUM))

free_area_t free_area[MAX_BUDDY_ORDER + 1]; // 0 ~ MAX_BUDDY_ORDER
frame_array_t * frame_array;


void memory_reserve(unsigned int start,unsigned int end) {
	unsigned int i;
	unsigned int start_index = (start - BUDDY_BASE_ADDRESS) / PAGE_SIZE; 
	unsigned int end_index = (end - BUDDY_BASE_ADDRESS) / PAGE_SIZE;
	uart_printf("strat:%x end:%x\n", start,end);
	uart_printf("strat_index:%d end_index:%d\n", start_index,end_index);
	for( i = start_index ; i <= end_index ; ++i ){
		if(frame_array[i].next->is_used == 0){
			(frame_array[i].next)->is_used = -1;
			free_area[0].num_free_page -= 1;
			list_del_entry( (struct list_head *) frame_array[i].next );
		}
	}
	uart_printf("reserve start_index: %d end_index: %d\n\n", start_index, end_index);
}

void memory_reserve_by_page_index(unsigned int start_index,unsigned int end_index) {
	unsigned int i;
	uart_printf("strat_index:%d end_index:%d\n", start_index,end_index);
	for( i = start_index ; i <= end_index ; ++i ){
		if(frame_array[i].next->is_used == 0){
			(frame_array[i].next)->is_used = -1;
			free_area[0].num_free_page -= 1;
			list_del_entry( (struct list_head *) frame_array[i].next );
		}
	}
	uart_printf("reserve start_index: %d end_index: %d\n\n", start_index, end_index);
}

void buddy_init(){
	int i;
	uart_printf("page_array size %d\n" ,sizeof(frame_array_t) * PAGE_FRAMES_NUM);
	
	//frame_array
	frame_array = simple_alloc(sizeof(frame_array_t) * PAGE_FRAMES_NUM);
	
	//init free_area
	for(i = 0; i <= MAX_BUDDY_ORDER ; ++i){
		INIT_LIST_HEAD(&free_area[i].free_page_list);
		free_area[i].order = i;
		free_area[i].num_free_page = 0;
	}
	
	//init frame_array
	//alloc num of PAGE_FRAMES_NUM of 2^0 page at free_area[0]
	//uart_printf("%d\n",PAGE_FRAMES_NUM);
	for(i = 0 ; i < PAGE_FRAMES_NUM; i++){
		free_page_t * page;
		page = simple_alloc(sizeof(free_page_t));
		
		INIT_LIST_HEAD(&page->head);
		page->index = i;
		page->order = 0;
		page->is_used = 0; // is_used = 0 mean allocable
		frame_array[i].next = page;
		list_add_tail(&page->head,&free_area[0].free_page_list);
		free_area[0].num_free_page += 1; // num of free page
		//uart_printf("%d %d %x\n",i,frame_array[i].next->is_used,frame_array[i].next);
	}
	uart_printf("%d\n",PAGE_FRAMES_NUM);
	
	// reserve
	//memory_reserve(KERNEL_VIRTURE_BASE + 0x0000,  KERNEL_VIRTURE_BASE + 0x1000);//multicore boot
	//memory_reserve(&kernel_start,  &kernel_end); // kernel
	//memory_reserve(get_cpio_address(),  get_cpio_address() + 0x80000); //initramfs
	//memory_reserve(get_dtb_address(), get_dtb_address() + 0x100000); //devicetree
	//memory_reserve_by_page_index(258048, 262143); // for peripheral region (16MB)
	
	//bottom up
	for(i = 0 ; i < PAGE_FRAMES_NUM; i++){
		recursive_coalesce_buddy(frame_array[i].next);
	}
	uart_printf("done\n");
}

void print_buddy_info_log(){
	int i,count;
	for(i = MAX_BUDDY_ORDER ; i >= 0 ; --i){
	//for(i = 0 ; i <= MAX_BUDDY_ORDER ; ++i){
		count=0;
		uart_printf("Frame freelists: %d\n",i);
		uart_printf("num of free page in order %d: %d\n",i,free_area[i].num_free_page);
		struct list_head * pointer;
		uart_printf("page index: ");
		list_for_each(pointer,&free_area[i].free_page_list){
			if(((free_page_t *)pointer)->is_used==0){
				uart_printf("%d ",((free_page_t *)pointer)->index);
				count++;
			}
		}
		uart_printf("\n");
		uart_printf("%d\n",count);
		uart_printf("\n");
	}
}

void test_buddy_print(){

	uart_printf("%d \n", PAGE_FRAMES_NUM);
	uart_printf("%d \n", MAX_BUDDY_ORDER);

	print_buddy_info_log();
	
	print_used_list();
	
	print_reserve_list();

	void * ptr_1 = alloc_page(4096*8 + 1);
	uart_printf("block physical address: %x\n",ptr_1);
	print_buddy_info_log();
	void * ptr_2 = alloc_page(1);
	uart_printf("block physical address: %x\n",ptr_2);
	print_buddy_info_log();
	void * ptr_3 = alloc_page(4096*3 + 1);
	uart_printf("block physical address: %x\n",ptr_3);
	print_buddy_info_log();
	
	print_used_list();
	
	/*void * ptr_4 = alloc_page(4096*5 + 1);
	void * ptr_5 = alloc_page(4096*2);
	void * ptr_6 = alloc_page(4096*1);
	print_buddy_info_log();*/
	
	
	/*uart_printf("block physical address: %x\n",ptr_6);
	uart_printf("block physical address: %x\n",ptr_5);
	uart_printf("block physical address: %x\n",ptr_4);*/
	uart_printf("block physical address: %x\n",ptr_3);
	uart_printf("block physical address: %x\n",ptr_2);
	uart_printf("block physical address: %x\n",ptr_1);
	
	uart_printf("\n");
	
	/*free_page(ptr_6);
	free_page(ptr_5);
	free_page(ptr_4);*/
	
	free_page(ptr_3);
	print_buddy_info_log();
	free_page(ptr_2);
	print_buddy_info_log();
	free_page(ptr_1);
	
	print_buddy_info_log();
	
	print_used_list();
}

void page_split(int order,free_page_t * page){
	
	//uart_printf("\n--------------------------------------------------\n");
	//uart_printf("split page index %d in order %d\n" ,page->index , order);
	//remove page from order's list
	free_area[order].num_free_page -= 1;
	list_del_entry( (struct list_head *) page);
	
	//add page to (odder-1)'s list
	
	free_page_t * page_left;
	free_page_t * page_right;
	
	page_left = frame_array[page->index].next;
	page_right = frame_array[(page->index)^((int)pow(2,page->order-1))].next;
	
	page_left->order = order-1;
	page_left->is_used = 0;
	//uart_printf("new page_left_index: %d\n", page_left->index);
	
	//page_right->index = page->index ^ ((int)pow(2,page->order-1)); 
	page_right->order = order-1;
	page_right->is_used = 0;
	//uart_printf("new page_right_index: %d\n", page_right->index);
	// add page to free_area
	list_add_tail(&page_left->head, &free_area[order-1].free_page_list);
	list_add_tail(&page_right->head, &free_area[order-1].free_page_list);
	
	free_area[order-1].num_free_page += 2; // num of free page
	
	//uart_printf("--------------------------------------------------\n");
}

void page_coalesce(int order, free_page_t * page_left, free_page_t * page_right){
	int condi_1 = ((page_left->is_used == 0 ) && (page_right->is_used == 0 ));
	int condi_2 = ((page_left->index ^ (int)pow(2,page_left->order)) == page_right->index);

	if(condi_1 && condi_2){
		//remove page from order's list
		//uart_printf("--------------------------------------------------\n");
		//uart_printf("coalesce page index %d and %d in order %d\n" ,page_left->index ,page_right->index ,order);
		free_area[order].num_free_page -= 2;
		list_del_entry( (struct list_head *) page_left);
		list_del_entry( (struct list_head *) page_right);
		
		//add page to (odder+1)'s list
		
		free_page_t * page;
		
		page = frame_array[min(page_left->index,page_right->index)].next;
	
		page->order = order+1;
		page->is_used = 0;
			
		//uart_printf("new page_index: %x\n", page->index);
	
		list_add_tail(&page->head, &free_area[order+1].free_page_list);

		free_area[order+1].num_free_page += 1; // num of free page
		
		//uart_printf("--------------------------------------------------\n");
	}
	else{
		uart_printf("\n--------------------------------------------------\n");
		uart_printf("buddy allocate coalesce error!!!\n");
		uart_printf("--------------------------------------------------\n");
	}

}

void * alloc_page(unsigned int size){ // unit byte
	
	double proper_page = (double)(size) / (double)PAGE_SIZE;
	int proper_order = 0;
	while(pow(2,proper_order)<proper_page)proper_order++;
	//uart_printf("require size: %d proper_page: %f proper_order %d\n",size ,proper_page ,proper_order);
	
	// start look up from free_area[proper_order]
	if(free_area[proper_order].num_free_page == 0){
		recursive_find_buddy(proper_order+1);
	}
	
	free_page_t * page = (free_area[proper_order].free_page_list).next;

	// remove from free area and mark as used 
	page->is_used = 1;
	list_del_entry( (struct list_head *) page);
	free_area[proper_order].num_free_page -= 1; // num of free page
	
	//uart_printf("allocate page index: %d page order: %d\n\n",page->index ,page->order);
	
	// translate to memory address
	// block's physical address = block's index * 4096 + base address
	char * ptr = page->index * PAGE_SIZE + BUDDY_BASE_ADDRESS;
	int i,num_page=1;
	for(i=0;i<proper_order;i++)num_page*=2;
	for(i=0;i<num_page*PAGE_SIZE;i++){ // memzero
		*(ptr+i)=0;
	}
	return ptr;
}

void free_page(void * ptr){
	
	// find the page in the used list
	free_page_t * page = get_page_and_in_used_list(ptr);
	//uart_printf("address %x\n",ptr);
	//uart_printf("free page index: %d page order: %d\n",page->index ,page->order);
	
	if(page == NULL){
		uart_printf("error this address is not in the used list\n");
	}
	else{
		// add to free area and mark as used 
		page->is_used = 0;
		list_add_tail(&page->head, &free_area[page->order].free_page_list);
		free_area[page->order].num_free_page += 1; // num of free page
		
		recursive_coalesce_buddy(page);
		
	}
	
}

void recursive_find_buddy(int order){
	int condi_1 = (free_area[order].num_free_page==0);
	int condi_2 = (order+1 <= MAX_BUDDY_ORDER);
	if(condi_1 && condi_2){
		recursive_find_buddy(order+1);
	}
	page_split(order, (free_area[order].free_page_list).next);
}

void recursive_coalesce_buddy(free_page_t * page){

	int brother_index = page->index ^ (int)pow(2,page->order);
	
	free_page_t * page_left = frame_array[min(page->index,brother_index)].next;
	free_page_t * page_right = frame_array[max(page->index,brother_index)].next;
	
	int condi_1 = ((page_left->order == page_right->order) && page_left->order < MAX_BUDDY_ORDER);
	int condi_2 = ((page_left->is_used == 0) && (page_right->is_used == 0));
	
	if(condi_1 && condi_2){
		page_coalesce(page->order,page_left,page_right);
		recursive_coalesce_buddy(frame_array[page_left->index].next);
	}
	
}

void print_used_list(){
	int i;
	uart_printf("frame used list: \n");
	for(i = 0 ; i < PAGE_FRAMES_NUM; ++i){
		if((frame_array[i].next)->is_used == 1)
			uart_printf("page index: %d page order: %d\n",(frame_array[i].next)->index, (frame_array[i].next)->order);
	}
	uart_printf("\n");
}

void print_reserve_list(){
	int i;
	uart_printf("frame reserve list: \n");
	for(i = 0 ; i < PAGE_FRAMES_NUM; ++i){
		if((frame_array[i].next)->is_used == -1)
			uart_printf("page index: %d page order: %d\n",(frame_array[i].next)->index, (frame_array[i].next)->order);
	}
	uart_printf("\n");
}

free_page_t * get_page_and_in_used_list(void * ptr){
	// block's physical address = block's index * 4096 + base address
	// block's index = (block's physical address - base address) / 4096
	int index = ((int)ptr - BUDDY_BASE_ADDRESS) / PAGE_SIZE ;
	free_page_t * page = frame_array[index].next;
	return page;
}

int cal_page_order(unsigned int size){
	double proper_page = (double)(size) / (double)PAGE_SIZE;
	int proper_order = 0;
	while(pow(2,proper_order)<proper_page)proper_order++;
	//uart_printf("size %d, need order : %d\n",size,proper_order);
	return proper_order;
}

