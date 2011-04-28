#ifndef __SUBVERSIVE_CTL_H
#define __SUBVERSIVE_CTL_H

struct rk_args {
	long mode;
	long param1, param2, param3, param4, param5, param6;
	char pad[512];
};

enum {
	HIDE_INODE = 0,
	UNHIDE_INODE,
	LULZ_MODE,
	GET_ROOT,
	HIDE_PID,
	UNHIDE_PID,
};

#endif
