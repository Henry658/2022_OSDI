#define __ASSEMBLY__
#include "syscall.h"

.global getpid
getpid:
	mov x8, SYS_GETPID
	svc #0
	ret

.global uart_read
uart_read:
	mov x8, SYS_UART_READ
	svc #0
	ret

.global uart_write
uart_write:
	mov x8, SYS_UART_WRITE
	svc #0
	ret

.global exec
exec:
	mov x8, SYS_EXEC
	svc #0
	ret

.global fork
fork:
	mov x8, SYS_FORK
	svc #0
	ret

.global exit
exit:
	mov x8, SYS_EXIT
	svc #0

.global mbox_call
mbox_call:
	mov x8, SYS_MBOX_CALL
	svc #0
	ret

.global kill
kill:
	mov x8, SYS_KILL
	svc #0
	ret
	
	
.globl _exec_user
_exec_user:

	msr	elr_el1, x0
	ldr x0, =0x40000
	msr sp_el0, x0
	mov x0, 0x3c0
	msr spsr_el1, x0
	eret
