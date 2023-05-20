/*
  FUSE ssd: FUSE ioctl example
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "ssd_fuse_header.h"
#define SSD_NAME       "ssd_file"
#define get_pca_address(pca) (pca.fields.lba + pca.fields.nand * PAGE_PER_BLOCK) 
enum
{
    SSD_NONE,
    SSD_ROOT,
    SSD_FILE,
};

typedef enum{
		valid,
		use,
		stale
}page_state_t;

static size_t physic_size;
static size_t logic_size;
static size_t host_write_size;
static size_t nand_write_size;

typedef union pca_rule PCA_RULE;
union pca_rule
{
    unsigned int pca;
    struct
    {
        unsigned int lba : 16;
        unsigned int nand: 16;
    } fields;
};

int op_number = 1;
int curr_op = 0;

PCA_RULE curr_pca;
static unsigned int get_next_pca();
static int garbege_collection();

unsigned int* L2P,* P2L;
int *stale_count, *valid_count, *op_block;
page_state_t *page_state;

static int check_op(int block_idx){
	for(int i = 0 ; i < op_number ; i++){
		if(op_block[i] == block_idx){
			return 1;
		}
	}
	return 0;
}

static int ftl_set_stale(unsigned int pca){
	if(pca == INVALID_PCA){
		return -1;
	}
	PCA_RULE pca_temp;
	pca_temp.pca = pca;
	printf("set block: %d page: %d stale\n", pca_temp.fields.nand, pca_temp.fields.lba);
	page_state[get_pca_address(pca_temp)] = stale;
	stale_count[pca_temp.fields.nand]++;
	P2L[get_pca_address(pca_temp)] = INVALID_LBA;
	return 0;
}

static int ssd_resize(size_t new_size)
{
    //set logic size to new_size
    if (new_size > NAND_SIZE_KB * 1024)
    {
        return -ENOMEM;
    }
    else
    {
        logic_size = new_size;
        return 0;
    }

}

static int ssd_expand(size_t new_size)
{
    //logic must less logic limit

    if (new_size > logic_size)
    {
        return ssd_resize(new_size);
    }

    return 0;
}

static int nand_read(char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //read
    if ( (fptr = fopen(nand_name, "r") ))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fread(buf, 1, 512, fptr);
        fclose(fptr);
    }
    else
    {
        printf("open file fail at nand read pca = %d\n", pca);
        return -EINVAL;
    }
    return 512;
}
static int nand_write(const char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //write
    if ( (fptr = fopen(nand_name, "r+")))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fwrite(buf, 1, 512, fptr);
        fclose(fptr);
        physic_size ++;
        valid_count[my_pca.fields.nand]--;
    }
    else
    {
        printf("open file fail at nand (%s) write pca = %d, return %d\n", nand_name, pca, -EINVAL);
        return -EINVAL;
    }

    nand_write_size += 512;
    return 512;
}

static int nand_erase(int block_index)
{
    char nand_name[100];
    FILE* fptr;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, block_index);
    fptr = fopen(nand_name, "w");
    if (fptr == NULL)
    {
        printf("erase nand_%d fail", block_index);
        return 0;
    }
    fclose(fptr);
    valid_count[block_index] = PAGE_PER_BLOCK;
    stale_count[block_index] = 0;
    return 1;
}

static unsigned int get_next_pca()
{    
	int temp = get_valid_page();
	if(temp == OUT_OF_BLOCK){
		garbege_collection();
	}else{
	curr_pca.pca = temp;
	}
	return curr_pca.pca;
}

static int ftl_read( char* buf, size_t lba)
{
    // TODO done
    if(L2P[lba] == INVALID_PCA){
    	memset(buf, 0, 512);
    	return 512;
    }
    return nand_read(buf, L2P[lba]);
    
}

static int ftl_write(const char* buf, size_t lba_rnage, size_t lba)
{
    // TODO check
    PCA_RULE pca_check;
    pca_check.pca = L2P[lba];
    
    if((pca_check.pca != INVALID_PCA)){// page not clear, it's stale
    	printf("want to write block: %d page: %d\n", pca_check.fields.nand, pca_check.fields.lba);
    	ftl_set_stale(pca_check.pca);
    }
    int pca = get_next_pca();
    if(pca == INVALID_PCA) return -1;
    int res = nand_write(buf,pca);
    L2P[lba] = pca;
    PCA_RULE pca_rule;
	pca_rule.pca = pca;
  	printf("write block: %d page: %d\n", pca_rule.fields.nand, pca_rule.fields.lba);
  	page_state[get_pca_address(pca_rule)] = use;
  	P2L[get_pca_address(pca_rule)] = lba;
  	return res;
}

static int ssd_file_type(const char* path)
{
    if (strcmp(path, "/") == 0)
    {
        return SSD_ROOT;
    }
    if (strcmp(path, "/" SSD_NAME) == 0)
    {
        return SSD_FILE;
    }
    return SSD_NONE;
}
static int ssd_getattr(const char* path, struct stat* stbuf,
                       struct fuse_file_info* fi)
{
    (void) fi;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = time(NULL);
    switch (ssd_file_type(path))
    {
        case SSD_ROOT:
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            break;
        case SSD_FILE:
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = logic_size;
            break;
        case SSD_NONE:
            return -ENOENT;
    }
    return 0;
}
static int ssd_open(const char* path, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_NONE)
    {
        return 0;
    }
    return -ENOENT;
}
static int ssd_do_read(char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, rst ;
    char* tmp_buf;

    //off limit
    if ((offset ) >= logic_size)
    {
        return 0;
    }
    if ( size > logic_size - offset)
    {
        //is valid data section
        size = logic_size - offset;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;
    tmp_buf = calloc(tmp_lba_range * 512, sizeof(char));

    for (int i = 0; i < tmp_lba_range; i++) {
        // TODO done
        ftl_read(tmp_buf + i * 512, tmp_lba + i);
    }

    memcpy(buf, tmp_buf + offset % 512, size);

    free(tmp_buf);
    return size;
}
static int ssd_read(const char* path, char* buf, size_t size,
                    off_t offset, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_read(buf, size, offset);
}
static int ssd_do_write(const char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, process_size;
    int idx, curr_size, remain_size, rst;
    char* tmp_buf;

    host_write_size += size;
    if (ssd_expand(offset + size) != 0)
    {
        return -ENOMEM;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;

    process_size = 0;
    remain_size = size;
    curr_size = 0;
    
    for (idx = 0; idx < tmp_lba_range; idx++)
    {
        // TODO ing
        tmp_buf = malloc(512 * sizeof(char));
        memset(tmp_buf, 0, 512 * sizeof(char));
        if( (idx == 0) && ((offset%512) != 0)){
        	int start = 0, end = 512;
        	ftl_read(tmp_buf, tmp_lba + idx);
        	start = (offset)%512;
        	for(int i = start ; i<end; i++){
        		tmp_buf[i] = buf[process_size++];
        	}
        	ftl_write(tmp_buf, 512, tmp_lba + idx);
        }
        else if((idx == tmp_lba_range-1) && (((offset+size)%512) != 0)){
        	int start = 0, end = 512;
        	ftl_read(tmp_buf, tmp_lba + idx);
        	end = (offset+size)%512;
        	for(int i = start ; i<end; i++){
        		tmp_buf[i] = buf[process_size++];
        	}
        	ftl_write(tmp_buf, 512, tmp_lba + idx);
        }
        else{
        	int start = 0, end = 512;
        	for(int i = start ; i<end; i++){
        		tmp_buf[i] = buf[process_size++];
        	}
        	ftl_write(tmp_buf, 512, tmp_lba + idx);
    	}
    	free(tmp_buf);
    }
    return size;
}
static int ssd_write(const char* path, const char* buf, size_t size,
                     off_t offset, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_write(buf, size, offset);
}

int get_valid_page(){
	PCA_RULE pca; // nand = block , lba = page
	for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		if(check_op(i))continue;
		for(int j = 0 ; j < PAGE_PER_BLOCK ; j++){
			if(page_state[i * PAGE_PER_BLOCK + j] == valid){
				pca.fields.nand = i;
				pca.fields.lba = j;
				return pca.pca;
			}
		}
	}
	return OUT_OF_BLOCK;
}

static PCA_RULE find_valid_page(){
	PCA_RULE pca; // nand = block , lba = page
	for(int i = 0 ; i < PAGE_PER_BLOCK ; i++){
		if(page_state[op_block[curr_op] * PAGE_PER_BLOCK + i] == valid){
			pca.fields.lba = i;
			pca.fields.nand = op_block[curr_op];
			break;
		}
	}
	return pca;
}

static int garbege_collection(){
	// find the block which has most garbege page (state).
	printf("start garbege collection\n");
	PCA_RULE victim_pca, new_free_page; 
	int victim_block_idx = PHYSICAL_NAND_NUM+1;
	int max = -1;
	char buf[512];
	
	for(int i = 0; i < PHYSICAL_NAND_NUM; i++){
		if(check_op(i))continue;
		if(max <= stale_count[i]){
			max = stale_count[i];
			victim_block_idx = i;
		}
	}
	
	if(victim_block_idx == PHYSICAL_NAND_NUM+1){
		printf("do grabege collection fail\n");
		return -1; //error
	}
	
	// traversal the victim block, and find the using page that need to be copied.
	for(int i = 0; i < PAGE_PER_BLOCK; i++){
		victim_pca.fields.lba = i;
		victim_pca.fields.nand = victim_block_idx;
		if(page_state[victim_block_idx * PAGE_PER_BLOCK + i] == use){ //use, need to copy
			// find the vaild page, and copy the using page to it.
			new_free_page = find_valid_page(); // get free page
			memset(buf,0,512); //init buf
			
			nand_read(buf, victim_pca.pca); // read vaild page on victim block  
			nand_write(buf, new_free_page.pca); // write vaild page to new free page
			page_state[get_pca_address(new_free_page)] = use;
			
			P2L[get_pca_address(new_free_page)] = P2L[get_pca_address(victim_pca)];
			L2P[P2L[get_pca_address(victim_pca)]] = new_free_page.pca;
			P2L[get_pca_address(victim_pca)] = INVALID_LBA;
		}else{
			P2L[get_pca_address(victim_pca)] = INVALID_LBA;
		}
	}
	nand_erase(victim_block_idx);
	op_block[curr_op] = victim_block_idx;
	curr_op = (curr_op + 1) % op_number;
	
	for(int i = 0 ; i < PAGE_PER_BLOCK ; i++){
		page_state[victim_pca.fields.nand * PAGE_PER_BLOCK + i] = valid;
		P2L[victim_pca.fields.nand * PAGE_PER_BLOCK + i] = INVALID_LBA;
	}
	
	for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		if(check_op(i))continue;
		for(int j = 0; j < PAGE_PER_BLOCK ; j++){
			if(page_state[i * PAGE_PER_BLOCK + j] == valid){
				curr_pca.fields.nand = i;
				curr_pca.fields.lba = j;
				return curr_pca.pca;
			}
		}
	}
	printf("do grabege collection done\n");
	return 0;
}

static int ssd_truncate(const char* path, off_t size,
                        struct fuse_file_info* fi)
{
    (void) fi;
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    curr_pca.pca = 0;

	for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		valid_count[i] = PAGE_PER_BLOCK;
		stale_count[i] = 0;
	}
	
    for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		for(int j = 0 ; j < PAGE_PER_BLOCK ; j++){
			page_state[i * PAGE_PER_BLOCK + j] = valid;
		}
	}
	
	for(int i = 0 ; i < op_number ; i++){
		op_block[i] = PHYSICAL_NAND_NUM - op_number + i ;
	}
    
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }

    return ssd_resize(size);
}
static int ssd_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info* fi,
                       enum fuse_readdir_flags flags)
{
    (void) fi;
    (void) offset;
    (void) flags;
    if (ssd_file_type(path) != SSD_ROOT)
    {
        return -ENOENT;
    }
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, SSD_NAME, NULL, 0, 0);
    return 0;
}
static int ssd_ioctl(const char* path, unsigned int cmd, void* arg,
                     struct fuse_file_info* fi, unsigned int flags, void* data)
{

    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    if (flags & FUSE_IOCTL_COMPAT)
    {
        return -ENOSYS;
    }
    switch (cmd)
    {
        case SSD_GET_LOGIC_SIZE:
            *(size_t*)data = logic_size;
            return 0;
        case SSD_GET_PHYSIC_SIZE:
            *(size_t*)data = physic_size;
            return 0;
        case SSD_GET_WA:
            *(double*)data = (double)nand_write_size / (double)host_write_size;
            return 0;
    }
    return -EINVAL;
}
static const struct fuse_operations ssd_oper =
{
    .getattr        = ssd_getattr,
    .readdir        = ssd_readdir,
    .truncate       = ssd_truncate,
    .open           = ssd_open,
    .read           = ssd_read,
    .write          = ssd_write,
    .ioctl          = ssd_ioctl,
};
int main(int argc, char* argv[])
{
    int idx;
    char nand_name[100];
    
    physic_size = 0;
    logic_size = 0;
    curr_pca.pca = INVALID_PCA;

    L2P = malloc(LOGICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    P2L = malloc(PHYSICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    valid_count = malloc(PHYSICAL_NAND_NUM * sizeof(int));
	page_state = (page_state_t *)malloc(PHYSICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(page_state_t));
	stale_count = malloc(PHYSICAL_NAND_NUM * sizeof(int));
	op_block = malloc(op_number * sizeof(int));
	
	for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		valid_count[i] = PAGE_PER_BLOCK;
		stale_count[i] = 0;
	}
	// init page state
	for(int i = 0 ; i < PHYSICAL_NAND_NUM ; i++){
		for(int j = 0 ; j < PAGE_PER_BLOCK ; j++){
			page_state[i * PAGE_PER_BLOCK + j] = valid;
		}
	}
	// init op block
	for(int i = 0 ; i < op_number ; i++){
		op_block[i] = PHYSICAL_NAND_NUM - op_number + i ;
	}
	
    //create nand file
    for (idx = 0; idx < PHYSICAL_NAND_NUM; idx++)
    {
        FILE* fptr;
        snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, idx);
        fptr = fopen(nand_name, "w");
        if (fptr == NULL)
        {
            printf("open fail");
        }
        fclose(fptr);
    }
    // init
    curr_pca.pca = 0;
    
    return fuse_main(argc, argv, &ssd_oper, NULL);
}
