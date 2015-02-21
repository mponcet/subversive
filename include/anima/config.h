#ifndef __CONFIG_H
#define __CONFIG_H

#define MAX_HIDDEN_INODES 1024
#define MAX_HIDDEN_PIDS   1024

struct rootkit_config {
	int state;
#define RK_BOOT		0
#define RK_ACTIVE	1
#define RK_SHUTDOWN	2
	int dr_protect;		/* DR access protection */
	int patch_debug;	/* 1 => patch_debug, 0 => die_notifier */
};
extern struct rootkit_config rk_cfg;

#endif
