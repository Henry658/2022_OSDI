#ifndef _INITRAMFS_H
#define _INITRAMFS_H

#include "typedef.h"
#include "vfs.h"

#define EOF           (-1)
#define TMP_FILE_SIZE 4096

int initramfs_register();

int initramfs_setup_mount(filesystem_t * fs, struct mount* mount);


dentry_t * initramfs_create_dentry(dentry_t * parent, const char* name, int type);

vnode_t * initramfs_create_vnode(dentry_t * dentry);

//----------------------------------------------------------------------------------


int initramfs_v_lookup(vnode_t* dir, vnode_t** target, const char* component_name);

int initramfs_v_create(vnode_t* dir, vnode_t** target, const char* component_name);

int initramfs_v_mkdir(vnode_t* dir, vnode_t** target, const char* component_name);

int initramfs_ls(vnode_t* dir);

//----------------------------------------------------------------------------------
	
int initramfs_f_write(file_t* file, void* buf, uint64_t len);

int initramfs_f_read(file_t* file, void* buf, uint64_t len);

#endif
