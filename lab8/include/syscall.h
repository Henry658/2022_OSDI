#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS_GETPID        0
#define SYS_UART_READ     1
#define SYS_UART_WRITE    2
#define SYS_EXEC          3	
#define SYS_FORK          4		
#define SYS_EXIT          5
#define SYS_MBOX_CALL     6
#define SYS_KILL          7
#define SYS_VFS_OPEN      11
#define SYS_VFS_CLOSE     12
#define SYS_VFS_WRITE     13
#define SYS_VFS_READ      14
#define SYS_VFS_MKDIR     15
#define SYS_VFS_MOUNT     16
#define SYS_VFS_CHDIR     17

#endif

#ifndef __ASSEMBLY__

#include "typedef.h"
#include "exception.h"

extern uint64_t getpid();
extern uint32_t uart_read(char buf[], uint32_t size);
extern uint32_t uart_write(const char buf[], uint32_t size);
extern int exec(const char* name);
extern int fork();

void exec_user(void* addr);

void sys_getpid(trapframe_t * trapframe);

void sys_uart_read(trapframe_t * trapframe);

void sys_uart_write(trapframe_t * trapframe);

void sys_exec(trapframe_t * trapframe);

void sys_fork(trapframe_t * trapframe);

void sys_exit(trapframe_t * trapframe);

void sys_mbox_call(trapframe_t * trapframe);

void sys_kill(trapframe_t * trapframe);

#endif
