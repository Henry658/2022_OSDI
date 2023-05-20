#ifndef _TMPFS_H
#define _TMPFS_H

#include "typedef.h"
#include "vfs.h"

#define EOF           (-1)
#define TMP_FILE_SIZE 4096

typedef struct tmpfs_internal{
	int size;
	char buf[TMP_FILE_SIZE];
}tmpfs_internal_t;

int tmpfs_register();

int tmpfs_setup_mount(filesystem_t * fs, struct mount* mount);


dentry_t * tmpfs_create_dentry(dentry_t * parent, const char* name, int type);

vnode_t * tmpfs_create_vnode(dentry_t * dentry);

//----------------------------------------------------------------------------------


int tmpfs_v_lookup(vnode_t* dir, vnode_t** target, const char* component_name);

int tmpfs_v_create(vnode_t* dir, vnode_t** target, const char* component_name);

int tmpfs_v_mkdir(vnode_t* dir, vnode_t** target, const char* component_name);

int tmpfs_ls(vnode_t* dir);

//----------------------------------------------------------------------------------
	
int tmpfs_f_write(file_t* file, void* buf, uint64_t len);

int tmpfs_f_read(file_t* file, void* buf, uint64_t len);

#endif
