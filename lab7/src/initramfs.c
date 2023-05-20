#include "initramfs.h"
#include "vfs.h"
#include "alloc.h"
#include "cpio.h"
#include "mmu.h"



vnode_operations_t* initramfs_v_ops;
file_operations_t* initramfs_f_ops;

int initramfs_register(){
	
	//set the initramfs vnode operation
	initramfs_v_ops = (vnode_operations_t*)dynamic_alloc(sizeof(vnode_operations_t));
	initramfs_v_ops->lookup = initramfs_v_lookup;
	initramfs_v_ops->create = initramfs_v_create;
	
	//set the initramfs file operation
	initramfs_f_ops = (file_operations_t*)dynamic_alloc(sizeof(file_operations_t));
	
	initramfs_f_ops->write = initramfs_f_write;
	initramfs_f_ops->read = initramfs_f_read;
	
	return 0;
}

int initramfs_setup_mount(filesystem_t * fs, struct mount* mount){
	mount->fs = fs;
	mount->root = initramfs_create_dentry(NULL, "/initramfs", DIRECTORY);

	dentry_t* file_dentry = initramfs_create_dentry(mount->root, "vfs1.img", REGULAR_FILE);
	
	void * user_program_c = get_usr_program_address("vfs1.img");
	void * user_program = (long unsigned int)user_program_c | KERNEL_VIRTURE_BASE;
	long long filesize = get_usr_program_size("vfs1.img");
	
	file_dentry->vnode->internal = user_program;
	file_dentry->vnode->file_size = filesize;
	return 0;
}


dentry_t * initramfs_create_dentry(dentry_t * parent, const char* name, int type){

	dentry_t * dentry = (dentry_t *)dynamic_alloc(sizeof(dentry_t));
	
	strcpy(dentry->name, name);
	dentry->parent = parent;
	INIT_LIST_HEAD(&dentry->list);
	INIT_LIST_HEAD(&dentry->childs);
	
	if(parent != NULL){
		list_add_tail(&dentry->list, &parent->childs);
	}
	
	dentry->vnode = initramfs_create_vnode(dentry);
	dentry->mount_point = NULL;
	dentry->type = type;
	
	return dentry;
}

vnode_t * initramfs_create_vnode(dentry_t * dentry){
	vnode_t * vnode = (vnode_t *)dynamic_alloc(sizeof(vnode_t));
	
	vnode->dentry = dentry;
	vnode->v_ops = initramfs_v_ops;
	vnode->f_ops = initramfs_f_ops;
	
	return vnode;
}


//----------------------------------------------------------------------------------


int initramfs_v_lookup(vnode_t* dir, vnode_t** target, const char* component_name){
	// component_name is empty, return dir vnode
	if(!strcmp(component_name, "")){
		*target = dir;
		return 0;
	}
	// search component_name in dir
	struct list_head* pointer = &dir->dentry->childs;
	list_for_each(pointer, &dir->dentry->childs){
		dentry_t* dentry = list_entry(pointer, dentry_t, list);
		if(!strcmp(dentry->name, component_name)){
			*target = dentry->vnode;
			return 0;
		}
	}
	*target = NULL;
	return -1;
}

int initramfs_v_create(vnode_t* dir, vnode_t** target, const char* component_name){
	return -1;
}

int initramfs_ls(vnode_t* dir){
	struct list_head* pointer = &dir->dentry->childs;
	list_for_each(pointer, &dir->dentry->childs){
		dentry_t* dentry = list_entry(pointer, dentry_t, list);
		uart_printf("%s ", dentry->name);
	}
	uart_printf("\n");
	return 0; 
}

//----------------------------------------------------------------------------------
	
int initramfs_f_write(file_t* file, void* buf, uint64_t len){
	return -1;
}

int initramfs_f_read(file_t* file, void* buf, uint64_t len){
	void* file_node = (void*)file->vnode->internal;
	long long file_size = file->vnode->file_size;
	
	char* dest = (char*)buf;
	char* src = &file_node[file->f_pos];
	uint64_t i;
	for(i = 0; i < len && i < file_size ; i++){
		dest[i] = src[i];
	}
	return i;
}
