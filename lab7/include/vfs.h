#ifndef _VFS_H
#define _VFS_H

#include "typedef.h"
#include "list.h"

#define O_CREAT 00000100
#define REGULAR_FILE 0
#define DIRECTORY 1
#define NR_OPEN_DEFAULT 16

//#define NULL    ((void*)0)

struct mount* rootfs;

typedef struct mount {
  struct dentry* root;
  struct filesystem* fs;
}mount_t;

typedef struct dentry{// directory entry
	char name[32];
	struct dentry * parent;
	struct list_head list;
	struct list_head childs;
	struct vnode * vnode;
	int type;
	struct mount* mount_point;
}dentry_t;

typedef struct vnode {
  //struct mount* mount;
  struct dentry* dentry;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  long long file_size;
  void* internal;
}vnode_t;

typedef struct vnode_operations{
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
}vnode_operations_t;

typedef struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
}filesystem_t;

// file handle
typedef struct file {
  struct vnode* vnode;
  uint64_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
}file_t;

typedef struct file_operations {
  int (*write)(struct file* file, const void* buf, uint64_t  len);
  int (*read)(struct file* file, void* buf, uint64_t  len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  //long lseek64(file_t* file, long offset, int whence);
}file_operations_t;

typedef struct files_struct{
	struct file* fd_array[NR_OPEN_DEFAULT];// For file descriptor, max open fd is 16.
}files_struct_t;

int register_filesystem(filesystem_t* fs);

file_t* vfs_open(const char* pathname, int flags);

int vfs_close(file_t* file);

int vfs_write(file_t* file, const void* buf, uint64_t  len);

int vfs_read(file_t* file, void* buf, uint64_t  len);

int vfs_mkdir(const char* pathname);

int vfs_mount(const char* device, const char* mount_point, const char* filesystem);

int vfs_lookup(const char* pathname, vnode_t** target);

vfs_chdir(const char* pathname);

#endif
