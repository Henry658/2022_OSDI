#include "tmpfs.h"
#include "alloc.h"

vnode_operations_t* tmpfs_v_ops;
file_operations_t* tmpfs_f_ops;

int tmpfs_register(){
	//uart_printf("tmpfs %d\n",NULL);
	//set the tmpfs vnode operation
	tmpfs_v_ops = (vnode_operations_t*)dynamic_alloc(sizeof(vnode_operations_t));
	tmpfs_v_ops->lookup = tmpfs_v_lookup;
	tmpfs_v_ops->create = tmpfs_v_create;
	tmpfs_v_ops->mkdir = tmpfs_v_mkdir;
	
	//set the tmpfs file operation
	tmpfs_f_ops = (file_operations_t*)dynamic_alloc(sizeof(file_operations_t));
	
	tmpfs_f_ops->write = tmpfs_f_write;
	tmpfs_f_ops->read = tmpfs_f_read;
	
	return 0;
}

int tmpfs_setup_mount(filesystem_t * fs, struct mount* mount){
	mount->fs = fs;
	mount->root = tmpfs_create_dentry(NULL, "/", DIRECTORY);
	return 0;
}


dentry_t * tmpfs_create_dentry(dentry_t * parent, const char* name, int type){

	dentry_t * dentry = (dentry_t *)dynamic_alloc(sizeof(dentry_t));
	
	strcpy(dentry->name, name);
	dentry->parent = parent;
	INIT_LIST_HEAD(&dentry->list);
	INIT_LIST_HEAD(&dentry->childs);
	
	if(parent != NULL){
		list_add_tail(&dentry->list, &parent->childs);
	}
	
	dentry->vnode = tmpfs_create_vnode(dentry);
	dentry->mount_point = NULL;
	dentry->type = type;
	
	return dentry;
}

vnode_t * tmpfs_create_vnode(dentry_t * dentry){
	vnode_t * vnode = (vnode_t *)dynamic_alloc(sizeof(vnode_t));
	
	vnode->dentry = dentry;
	vnode->v_ops = tmpfs_v_ops;
	vnode->f_ops = tmpfs_f_ops;
	
	return vnode;
}


//----------------------------------------------------------------------------------


int tmpfs_v_lookup(vnode_t* dir, vnode_t** target, const char* component_name){
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

int tmpfs_v_create(vnode_t* dir, vnode_t** target, const char* component_name){
	// create tmpfs internal structure
	tmpfs_internal_t* file_node = (tmpfs_internal_t*)alloc_page(sizeof(tmpfs_internal_t));
	
	// create child dentry
	dentry_t* d_child = tmpfs_create_dentry(dir->dentry, component_name, REGULAR_FILE);
	d_child->vnode->internal = (void*)file_node;
	
	*target = d_child->vnode;
	return 0;
}

int tmpfs_v_mkdir(vnode_t* dir, vnode_t** target, const char* component_name){
	// create tmpfs internal structure
	tmpfs_internal_t* dir_node = (tmpfs_internal_t*)alloc_page(sizeof(tmpfs_internal_t));
	
	// create child dentry
	dentry_t* d_child = tmpfs_create_dentry(dir->dentry, component_name, DIRECTORY);
	d_child->vnode->internal = (void*)dir_node;
	
	*target = d_child->vnode;
}

int tmpfs_ls(vnode_t* dir){
	struct list_head* pointer = &dir->dentry->childs;
	list_for_each(pointer, &dir->dentry->childs){
		dentry_t* dentry = list_entry(pointer, dentry_t, list);
		uart_printf("%s ", dentry->name);
	}
	uart_printf("\n");
	return 0; 
}

//----------------------------------------------------------------------------------
	
int tmpfs_f_write(file_t* file, void* buf, uint64_t len){
	
	tmpfs_internal_t* file_node = (tmpfs_internal_t*)file->vnode->internal;
	
	char* dest = &file_node->buf[file->f_pos];
	char* src = (char*)buf;
	uint64_t i;
	for(i = 0; i < len && src[i] != "\0"; i++){
		dest[i] = src[i];
	}
	dest[i] = EOF;
	return i;
}

int tmpfs_f_read(file_t* file, void* buf, uint64_t len){
	tmpfs_internal_t* file_node = (tmpfs_internal_t*)file->vnode->internal;
	
	char* dest = (char*)buf;
	char* src = &file_node->buf[file->f_pos];
	uint64_t i;
	for(i = 0; i < len && src[i] != (uint8_t)EOF; i++){
		dest[i] = src[i];
	}
	return i;
}

