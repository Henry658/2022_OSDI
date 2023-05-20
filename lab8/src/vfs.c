#include "vfs.h"
#include "tmpfs.h"
#include "fat32.h"
#include "initramfs.h"
#include "alloc.h"
#include "string.h"
#include "sched.h"
#include "mmu.h"

void vfs_init(){
	rootfs_init();
	sd_mount();
}

void rootfs_init(){
	// create tmpfs
	filesystem_t * tmpfs = (filesystem_t *)dynamic_alloc(sizeof(filesystem_t));
	tmpfs->name = (char *)dynamic_alloc(sizeof(char) * 6);	
	strcpy(tmpfs->name,"tmpfs");
	tmpfs->setup_mount = tmpfs_setup_mount;
	register_filesystem(tmpfs);
	
	//rootfs
	rootfs = (struct mount*)dynamic_alloc(sizeof(struct mount));
	tmpfs->setup_mount(tmpfs, rootfs);
	
	filesystem_t * initramfs = (filesystem_t *)dynamic_alloc(sizeof(filesystem_t));
	initramfs->name = (char *)dynamic_alloc(sizeof(char) * 11);	
	strcpy(initramfs->name,"initramfs");
	initramfs->setup_mount = initramfs_setup_mount;
	register_filesystem(initramfs);
	
	
	char mountpoint[11] = "/initramfs";
    vfs_mkdir(mountpoint);
    vfs_mount("initramfs", mountpoint, "initramfs");
}

int register_filesystem(struct filesystem* fs) {
// register the file system to the kernel.
// you can also initialize memory pool of the file system here.
	if(!strcmp(fs->name,"tmpfs")){
		uart_printf("Register tmpfs\n");
		return tmpfs_register();
	}
	
	if(!strcmp(fs->name,"initramfs")){
		uart_printf("Register initramfs\n");
		return initramfs_register();
	}
	
	if(!strcmp(fs->name,"fat32")){
		uart_printf("Register fat32\n");
		return fat32_register();
	}
	uart_printf("%s\n",fs->name);
	return -1;
}

file_t* create_fd(vnode_t * target){//create file descriptor
	file_t* fd = (file_t*)dynamic_alloc(sizeof(file_t));
	
	fd->vnode = target;
	fd->f_pos = 0;
	fd->f_ops = target->f_ops;
	return fd;
}

//-----------------------------------------------------------------------------------

void traversal(const char* pathname, vnode_t** target_node, char* target_path){//target_path(empty buf)
	uart_printf("%s\n", pathname);
	if(pathname[0] == '/'){ // absolute path
		vnode_t* root_node = rootfs->root->vnode; //mount->dentry->vnode;
		traversal_recursive(root_node->dentry, pathname+1, target_node, target_path);
	}
	else{ // relative path
		task_struct_t * current_c = get_current_task();
		task_struct_t * current = (long unsigned int)current_c | KERNEL_VIRTURE_BASE;
		
		vnode_t* root_node = current->pwd->vnode; //current->pwd(dentry)->vnode;
		traversal_recursive(root_node->dentry, pathname, target_node, target_path);
	}
	
}

void traversal_recursive(dentry_t *dentry, const char* path, vnode_t** target_node, char* target_path){
	int i = 0;
	while(path[i]){
		if(path[i] == '/') break;
		target_path[i] = path[i];
		i++;
	}
	target_path[i++] = '\0';
	*target_node = dentry->vnode;
	
	if(!strcmp(target_path,"")){
		//uart_printf("target find\n");
		return;
	}
	else if(!strcmp(target_path,".")){
		traversal_recursive(dentry, path + i, target_node, target_path);// ??
		return;
	}
	else if(!strcmp(target_path,"..")){
		if(dentry->parent == NULL){// root mount
			traversal_recursive(dentry, path + i, target_node, target_path);
			uart_printf("traverse: no parent\n");
			// should not be here??
			return;
		}
		else{
			traversal_recursive(dentry->parent, path + i, target_node, target_path);
			return;
		}
	}
	// find in node's child
	struct list_head *pointer;
	int found = 0;
	list_for_each(pointer, &dentry->childs){//traverse list
		dentry_t* curr_dentry = list_entry(pointer, dentry_t, list);
		if(!strcmp_2(curr_dentry->name, target_path)){
			if(curr_dentry->mount_point != NULL){
				traversal_recursive(curr_dentry->mount_point->root, path + i, target_node, target_path);
			}
			else if(curr_dentry->type == DIRECTORY){
				traversal_recursive(curr_dentry, path + i, target_node, target_path);
			}
			found = 1;
			break;
		}
	}
	if(!found){
		uart_printf("dentry->name: %s\n",dentry->name);
		int ret = dentry->vnode->v_ops->load_dentry(dentry, target_path);
		uart_printf("ret: %d\n", ret);
		if(ret == 0){
			traversal_recursive(dentry, path, target_node, target_path);
		}
	}
}

//-----------------------------------------------------------------------------------

file_t* vfs_open(const char* pathname, int flags) {
	uart_printf("here 0 \n");
	uart_printf("%s\n",pathname);
	// 1. Lookup pathname
	//    Find target_dir node and target_path based on pathname
	vnode_t * target_dir; // target vnode
	char target_path[128]; //target_path(empty buf)
	traversal(pathname, &target_dir, target_path);
	// 2. Create a new file handle for this vnode if found.
	vnode_t* target_file;
	if(target_dir->v_ops->lookup(target_dir, &target_file, target_path) == 0){
		uart_printf("here 1 \n");
		return create_fd(target_file);
	}	
	// 3. Create a new file if O_CREAT is specified in flags and vnode not found
	else{
		if(flags & O_CREAT){
			//uart_printf("target_dir->name: %s\n",target_dir->name);
			int result = target_dir->v_ops->create(target_dir, &target_file, target_path);
			if(result < 0) return NULL;
			target_file->dentry->type = REGULAR_FILE;
			uart_printf("here 2 \n");
			return create_fd(target_file);
		}
		else{
			uart_printf("error ??\n");
			return NULL;
		}
	
	}
	// lookup error code shows if file exist or not or other error occurs
	// 4. Return error code if fails
}

int vfs_close(file_t* file) {
	// 1. release the file handle
	dynamic_free((void *)file);
	return 0;
	// 2. Return error code if fails

}

int vfs_write(file_t* file, const void* buf, uint64_t  len) {

	if(file->vnode->dentry->type != REGULAR_FILE){
		uart_printf("Write on non regular file\n");
		return -1;
	}
	// 1. write len byte from buf to the opened file.
	// 2. return written size or error code if an error occurs.
	int res = file->f_ops->write(file, buf, len);
	file->f_pos += res;
	return res;
}

int vfs_read(file_t* file, void* buf, uint64_t  len) {
	if(file->vnode->dentry->type != REGULAR_FILE){
		uart_printf("Read on non regular file\n");
		return -1;
	}	
	// 1. read min(len, readable size) byte to buf from the opened file.
	// 2. block if nothing to read for FIFO type
	// 2. return read size or error code if an error occurs.
	int res = file->f_ops->read(file, buf, len);
	file->f_pos += res;
	return res;
}

int vfs_ls(file_t* fd){
	;//return fd->vnode->v_ops->ls(fd->vnode);
}


int vfs_mkdir(const char* pathname){
	vnode_t* target_dir;
	char child_name[256]; // For VFS the max pathanme length is 255.

	traversal(pathname, &target_dir, child_name);
	
	vnode_t* child_dir;
	int result = target_dir->v_ops->mkdir(target_dir, &child_dir, child_name);
	
	if(result < 0 ) return result; // error
	child_dir->dentry->type = DIRECTORY;
	
	return 0;
}

int vfs_chdir(const char* pathname){
	vnode_t* target_dir;
	char path_remain[256];
	traversal(pathname, &target_dir, path_remain);
	if(strcmp(path_remain, "")){
		return -1;
	}
	else{
		task_struct_t * current_c = get_current_task();
		task_struct_t * current = (long unsigned int)current_c | KERNEL_VIRTURE_BASE;
		current->pwd = target_dir->dentry;
		return 0;
	}
}

// error: -1: directory, -2: not found
int vfs_mount(const char* device, const char* mount_point, const char* filesystem){
	// check mountpoint is valid
	vnode_t* mount_dir;
	char path_remain[256];
	traversal(mount_point, &mount_dir, path_remain);
	//uart_printf("remain: %s\n",path_remain);
	if(!strcmp(path_remain, "")){
		if(mount_dir->dentry->type != DIRECTORY){
			uart_printf("error -1\n");
			return -1;
		}
	}
	else{
		uart_printf("error -2\n");
		return -2;
	}

	
	//mount fs on mount_point
	mount_t* mt = (mount_t*)dynamic_alloc(sizeof(mount_t));
	if(!strcmp(filesystem, "tmpfs")){
		filesystem_t* tmpfs = (filesystem_t*)dynamic_alloc(sizeof(filesystem_t));
		tmpfs->name = (char *)dynamic_alloc(sizeof(char) * strlen(filesystem));
		strcpy(tmpfs->name, filesystem);
		register_filesystem(tmpfs);//because tmpfs is root file system
		tmpfs->setup_mount = tmpfs_setup_mount;
		tmpfs->setup_mount(tmpfs, mt);
		mount_dir->dentry->mount_point = mt;
		mt->root->parent = mount_dir->dentry->parent;
	}
	
	if(!strcmp(filesystem, "initramfs")){
		filesystem_t* initramfs = (filesystem_t*)dynamic_alloc(sizeof(filesystem_t));
		initramfs->name = (char *)dynamic_alloc(sizeof(char) * strlen(filesystem));
		strcpy(initramfs->name,filesystem);
		//register_filesystem(initramfs);
		initramfs->setup_mount = initramfs_setup_mount;
		initramfs->setup_mount(initramfs, mt);
		mount_dir->dentry->mount_point = mt;
		mt->root->parent = mount_dir->dentry->parent;
	}
	
	if(!strcmp(filesystem, "fat32")){
		filesystem_t* fat32 = (filesystem_t*)dynamic_alloc(sizeof(filesystem_t));
		fat32->name = (char *)dynamic_alloc(sizeof(char) * strlen(filesystem));
		strcpy(fat32->name, filesystem);
		register_filesystem(fat32);
		fat32->setup_mount = fat32_setup_mount;
		fat32->setup_mount(fat32, mt);
		mount_dir->dentry->mount_point = mt;
		mt->root->parent = mount_dir->dentry->parent;
	}
	
	
	
	return 0;
}

int vfs_lookup(const char* pathname, vnode_t** target){
}

