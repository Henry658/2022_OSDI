#include "typedef.h"
#include "sched.h"
#include "mmu.h"
#include "mm.h"

uint64_t virtual_to_physical(uint64_t virt_addr){
    return (virt_addr << 16) >> 16;
}

uint64_t phy_to_pfn(uint64_t phy_addr){
	return phy_addr >> 12;
}

void memzero_c(uint8_t* addr, int size){
	for(int i = 0 ; i < size ; i++){
		*(addr + i) = 0;
	}
}

void memcpy(void *dest, void *src, uint64_t size){
	uint8_t *csrc = (uint8_t *)src;
	uint8_t *cdest = (uint8_t *)dest;
	
	for(uint64_t i = 0; i < size; i++){
		cdest[i] = csrc[i];
	}

}

// create pgd, return pgd address
void* create_pgd(task_struct_t* task){
	if(!task->mm.pgd){
		void * page = alloc_page(4096); // buddy alloc
		if(page == NULL) return NULL;
		//task->mm.pgd = (uint64_t)page;
		task->mm.pgd = virtual_to_physical((uint64_t)page);
	}
	return (void*) (task->mm.pgd + KERNEL_VIRTURE_BASE);

}

// create page table, return next level table
void* create_page_table(uint64_t* table, uint64_t idx){
	if(table == NULL) return NULL;
	//if(!table[idx]){
		void* page = alloc_page(4096); // buddy alloc
		if (page == NULL) return NULL;
		table[idx] = virtual_to_physical((uint64_t)page) | PD_TABLE;
	//}
	return (void*)((table[idx] & PAGE_MASK) + KERNEL_VIRTURE_BASE);
}

// create page, return page address
void* create_page(uint64_t* pte, uint64_t idx, int size){
	if(pte == NULL) return NULL;
	//if(!pte[idx]){
		void* page = alloc_page(size);
		if(page == NULL) return NULL;
		//uart_printf("0x%x 0x%x\n",PTE_NORAL_ATTR,ACCESS_PERM_RW);
		pte[idx] = virtual_to_physical((uint64_t)page) | PTE_NORAL_ATTR | ACCESS_PERM_RW;
		//uart_printf("0x%x\n",pte[idx]);
	//}
	return (void*)((pte[idx] & PAGE_MASK) + KERNEL_VIRTURE_BASE);
}

void map_page_(uint64_t* pte,void* page_virtual_address ,int page_order ,uint64_t idx){
		int i,num_page=1;
		for(i=0;i<page_order;i++)num_page*=2;
		void* page = virtual_to_physical(page_virtual_address);
		for(i = 1 ; i < num_page ; i++){
			//uart_printf("%d\n",i);
			//uart_printf("0x%x 0x%x\n",PTE_NORAL_ATTR,ACCESS_PERM_RW);
			pte[idx+i] = page + (i * 4096); 
			pte[idx+i] = pte[idx+i] | PTE_NORAL_ATTR | ACCESS_PERM_RW;
			//uart_printf("0x%x\n",pte[idx+i]);
		}
}

// allocate new page in user's address table, return page's viryeual address
uint64_t* map_page(task_struct_t* task, uint64_t user_addr, int size){
	uint64_t pgd_idx = (user_addr & (PD_MASK << PGD_SHIFT)) >> PGD_SHIFT;
	uint64_t pud_idx = (user_addr & (PD_MASK << PUD_SHIFT)) >> PUD_SHIFT;
	uint64_t pmd_idx = (user_addr & (PD_MASK << PMD_SHIFT)) >> PMD_SHIFT;
	uint64_t pte_idx = (user_addr & (PD_MASK << PTE_SHIFT)) >> PTE_SHIFT;
	
	uint64_t* pgd = create_pgd(task);
	uint64_t* pud = create_page_table(pgd, pgd_idx);
	uint64_t* pmd = create_page_table(pud, pud_idx);
	uint64_t* pte = create_page_table(pmd, pmd_idx);
	
	int page_order = cal_page_order(size);
	//uart_printf("file page order %d\n",page_order);
	
	uint64_t* page_virtual_address = create_page(pte, pte_idx, size);
	
	if(page_order){// for page order > 0
		map_page_(pte, page_virtual_address, page_order, pte_idx);
	}
	//uart_printf("done map page\n");
	return page_virtual_address;
}

void fork_pgd(task_struct_t* parent, task_struct_t* child){

	uint64_t* parent_pgd = (uint64_t*)((parent->mm.pgd & PAGE_MASK) | KERNEL_VIRTURE_BASE);
	uint64_t* child_pgd = create_pgd(child);
	for(int pgd_idx = 0; pgd_idx < 512; pgd_idx++){
		if(parent_pgd[pgd_idx]){
			//uart_printf("1\n");
			uint64_t* parent_pud = (uint64_t*)((parent_pgd[pgd_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE);
			uint64_t* child_pud = create_page_table(child_pgd, pgd_idx);
			fork_pud(parent_pud,child_pud);
		}
	}
	map_page_for_mmio(child_pgd, 0x3C000000, 0x3000000/PAGE_SIZE);
}

void fork_pud(uint64_t* parent_pud, uint64_t* child_pud){
	for(int pud_idx = 0; pud_idx < 512; pud_idx++){
		if(parent_pud[pud_idx]){
			//uart_printf("2\n");
			uint64_t* parent_pmd = (uint64_t*)((parent_pud[pud_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE);
			uint64_t* child_pmd = create_page_table(child_pud, pud_idx);
			fork_pmd(parent_pmd,child_pmd);
		}
	}
}

void fork_pmd(uint64_t* parent_pmd, uint64_t* child_pmd){
	for(int pmd_idx = 0; pmd_idx < 512; pmd_idx++){
		if(parent_pmd[pmd_idx]){
			//uart_printf("3\n");
			uint64_t* parent_pte = (uint64_t*)((parent_pmd[pmd_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE);
			uint64_t* child_pte = create_page_table(child_pmd, pmd_idx);
			fork_pte(parent_pte,child_pte);
		}
	}
}

void fork_pte(uint64_t* parent_pte, uint64_t* child_pte){
	for(int pte_idx = 0; pte_idx < 512; pte_idx++){
		//uart_printf("4\n");
		if(parent_pte[pte_idx]){
			uint64_t* parent_page = (uint64_t*)((parent_pte[pte_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE);
			uint64_t* child_page = create_page(child_pte, pte_idx, PAGE_SIZE);
			memcpy((void*)child_page, (void*)parent_page, PAGE_SIZE);
		}
	}
}

void map_page_for_mmio(uint64_t * pgd, uint64_t start_addr, int size) {
	for(int i = 0; i < size; ++i){
		helper(pgd, start_addr + i * PAGE_SIZE);
	}
}

void helper(uint64_t * pgd, uint64_t start_addr){
	uint64_t * p = (uint64_t) pgd | KERNEL_VIRTURE_BASE;
	void * page;
	
	uint64_t pgd_idx = (start_addr & (PD_MASK << PGD_SHIFT)) >> PGD_SHIFT;
	uint64_t pud_idx = (start_addr & (PD_MASK << PUD_SHIFT)) >> PUD_SHIFT;
	uint64_t pmd_idx = (start_addr & (PD_MASK << PMD_SHIFT)) >> PMD_SHIFT;
	uint64_t pte_idx = (start_addr & (PD_MASK << PTE_SHIFT)) >> PTE_SHIFT;

    // check if idx exist or we make a new one
	if(!p[pgd_idx]){
		page = alloc_page(PAGE_SIZE);
		p[pgd_idx] = virtual_to_physical((uint64_t)page) | PD_TABLE;
	}

	p = (p[pgd_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE;

	if(!p[pud_idx]){
		page = alloc_page(PAGE_SIZE);
		p[pud_idx] = virtual_to_physical((uint64_t)page) | PD_TABLE;
	}
	p = (p[pud_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE;

	if(!p[pmd_idx]){
		page = alloc_page(PAGE_SIZE);
		p[pmd_idx] = virtual_to_physical((uint64_t)page) | PD_TABLE;
	}
	p = (p[pmd_idx] & PAGE_MASK) | KERNEL_VIRTURE_BASE;
	p[pte_idx] = start_addr | PTE_NORAL_ATTR | ACCESS_PERM_RW;
}

uint64_t walk(uint64_t * pgd, uint64_t user_addr){
	uint64_t * p = (uint64_t) pgd | KERNEL_VIRTURE_BASE;
	uint64_t pgd_idx = (user_addr & (PD_MASK << PGD_SHIFT)) >> PGD_SHIFT;
	uint64_t pud_idx = (user_addr & (PD_MASK << PUD_SHIFT)) >> PUD_SHIFT;
	uint64_t pmd_idx = (user_addr & (PD_MASK << PMD_SHIFT)) >> PMD_SHIFT;
	uint64_t pte_idx = (user_addr & (PD_MASK << PTE_SHIFT)) >> PTE_SHIFT;
	
	p = (p[pgd_idx] & ~0xFFF) | KERNEL_VIRTURE_BASE;
	
	p = (p[pud_idx] & ~0xFFF) | KERNEL_VIRTURE_BASE;
	
	p = (p[pmd_idx] & ~0xFFF) | KERNEL_VIRTURE_BASE;
	
	p = ((p[pte_idx] & ~0xFFF) | (0xFFF & user_addr)) | KERNEL_VIRTURE_BASE;
	
	return p;
}
