#ifndef __KSYMS_H
#define __KSYMS_H

#include <linux/kdebug.h>
#include <linux/notifier.h>

struct kernel_syms {
	/* x86 specific */
	unsigned long system_call;
	unsigned long sys_call_table;
	unsigned long sys_call_table_call;

	/* IA32 emulation */
	unsigned long ia32_sysenter;
	unsigned long ia32_sysenter_sys_call_table_call;
	unsigned long ia32_syscall;
	unsigned long ia32_syscall_sys_call_table_call;
	unsigned long ia32_sys_call_table;

	/* syscalls */
	long (*old_sys_chdir)(char *);
	long (*old_sys_stat)(char *, void *);
	long (*old_sys_lstat)(char *, void *);
	long (*old_sys_fstatat)(int, char *, void *, int);
	long (*old_sys_open)(char *, int, umode_t);
	long (*old_sys_getdents)(unsigned int, void *, unsigned int);
	long (*old_sys_getdents64)(unsigned int, void *, unsigned int);
	long (*old_sys_execve)(const char *filename, const char **argv,
				const char **envp);

	/* PID syscalls */
	long (*old_sys_getpgid)(pid_t);
	long (*old_sys_setpgid)(pid_t, pid_t);
	long (*old_sys_getsid)(pid_t);
	long (*old_sys_getpriority)(int, int);
	long (*old_sys_kill)(pid_t, int);
	long (*old_sys_sched_setscheduler)(pid_t, int, void *);
	long (*old_sys_sched_setparam)(pid_t, void *);
	long (*old_sys_sched_getscheduler)(pid_t);
	long (*old_sys_sched_getparam)(pid_t, void *);
	long (*old_sys_sched_setaffinity)(pid_t, size_t, void *);
	long (*old_sys_sched_getaffinity)(pid_t, size_t, void *);
	long (*old_sys_sched_rr_get_interval)(pid_t, void *);
	long (*old_sys_wait4)(pid_t, int *, int, void *);
	long (*old_sys_get_robust_list)(int, void **, size_t *);
	long (*old_sys_perf_event_open)(void *, pid_t, int, int,
					unsigned long);
	void (*old_sys_exit)(int);
	void (*old_sys_exit_group)(int);
	long (*old_sys_getpid)(void);
	long (*old_sys_getppid)(void);
	long (*old_sys_getpgrp)(void);
	long (*old_sys_waitid)(int, pid_t, void *, int, void *);
	long (*old_sys_uname)(void *);
	long (*old_sys_rt_tgsigqueueinfo)(pid_t, pid_t, int, void *);
	long (*old_sys_rt_sigqueueinfo)(int, int, void *);
	long (*old_sys_prlimit64)(pid_t, unsigned int, void *, void *);
	long (*old_sys_ptrace)(long, pid_t, void *, void *);
	long (*old_sys_migrate_pages)(pid_t, unsigned long,
					unsigned long *, unsigned long *);
	long (*old_sys_move_pages)(pid_t, unsigned long, void **,
					int *, int *, int);
	long (*old_sys_clone)(unsigned long flags, void *child_stack,
				void *ptid, void *ctid, void *regs);
	long (*old_sys_fork)(void);
	long (*old_sys_vfork)(void);

	/* reboot */
	int (*old_sys_reboot)(int magic, int magic2, int cmd, void *arg);

	/* kernel functions */
	void (*old_do_debug)(struct pt_regs *, long);

	/* kernel data */
	void *die_chain;

	/* kernel API */
	void (*on_each_cpu)(void (*)(void *), void *, int);
	unsigned long (*_copy_from_user)(void *, const void *, unsigned int);
	unsigned long (*_copy_to_user)(void *, const void *, unsigned int);
	int (*register_die_notifier)(struct notifier_block *);
	int (*unregister_die_notifier)(struct notifier_block *);
	void *(*filp_open)(const char *, int, umode_t);
	int (*filp_close)(void *, void *);
	void *(*vmalloc)(unsigned int);
	void *(*vfree)(void *);
};

extern struct kernel_syms ksyms;

unsigned long get_symbol_addr(const char *name);
int get_kernel_syms(void);

#endif
