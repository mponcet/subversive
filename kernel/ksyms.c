#include <linux/fs.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>

#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/x86.h>

struct kernel_syms ksyms;

int get_kernel_syms(void)
{
	unsigned long *sys_call_table;

	if (x86_get_kernel_syms())
		return -1;

	sys_call_table = (unsigned long *)ksyms.sys_call_table;

	ksyms.old_sys_stat =
		(void *)sys_call_table[__NR_stat];
	ksyms.old_sys_lstat =
		(void *)sys_call_table[__NR_lstat];
	ksyms.old_sys_fstatat =
		(void *)sys_call_table[__NR_newfstatat];
	ksyms.old_sys_chdir =
		(void *)sys_call_table[__NR_chdir];
	ksyms.old_sys_open =
		(void *)sys_call_table[__NR_open];
	ksyms.old_sys_getdents =
		(void *)sys_call_table[__NR_getdents];
	ksyms.old_sys_getdents64 =
		(void *)sys_call_table[__NR_getdents64];
	ksyms.old_sys_getpid =
		(void *)sys_call_table[__NR_getpid];
	ksyms.old_sys_getppid =
		(void *)sys_call_table[__NR_getppid];
	ksyms.old_sys_getpgrp =
		(void *)sys_call_table[__NR_getpgrp];
	ksyms.old_sys_getpgid =
		(void *)sys_call_table[__NR_getpgid];
	ksyms.old_sys_setpgid =
		(void *)sys_call_table[__NR_setpgid];
	ksyms.old_sys_getsid =
		(void *)sys_call_table[__NR_getsid];
	ksyms.old_sys_getpriority =
		(void *)sys_call_table[__NR_getpriority];
	ksyms.old_sys_kill =
		(void *)sys_call_table[__NR_kill];
	ksyms.old_sys_sched_setscheduler =
		(void *)sys_call_table[__NR_sched_setscheduler];
	ksyms.old_sys_sched_setparam =
		(void *)sys_call_table[__NR_sched_setparam];
	ksyms.old_sys_sched_getscheduler =
		(void *)sys_call_table[__NR_sched_getscheduler];
	ksyms.old_sys_sched_getparam =
		(void *)sys_call_table[__NR_sched_getparam];
	ksyms.old_sys_sched_setaffinity =
		(void *)sys_call_table[__NR_sched_setaffinity];
	ksyms.old_sys_sched_getaffinity =
		(void *)sys_call_table[__NR_sched_getaffinity];
	ksyms.old_sys_sched_rr_get_interval =
		(void *)sys_call_table[__NR_sched_rr_get_interval];
	ksyms.old_sys_wait4 =
		(void *)sys_call_table[__NR_wait4];
	ksyms.old_sys_get_robust_list =
		(void *)sys_call_table[__NR_get_robust_list];
	ksyms.old_sys_exit =
		(void *)sys_call_table[__NR_exit];
	ksyms.old_sys_exit_group =
		(void *)sys_call_table[__NR_exit_group];
	ksyms.old_sys_uname =
		(void *)sys_call_table[__NR_uname];
	ksyms.old_sys_waitid =
		(void *)sys_call_table[__NR_waitid];
	ksyms.old_sys_rt_tgsigqueueinfo =
		(void *)sys_call_table[__NR_rt_tgsigqueueinfo];
	ksyms.old_sys_rt_sigqueueinfo =
		(void *)sys_call_table[__NR_rt_sigqueueinfo];
	ksyms.old_sys_prlimit64 =
		(void *)sys_call_table[__NR_prlimit64];
	ksyms.old_sys_ptrace =
		(void *)sys_call_table[__NR_ptrace];
	ksyms.old_sys_migrate_pages =
		(void *)sys_call_table[__NR_migrate_pages];
	ksyms.old_sys_move_pages =
		(void *)sys_call_table[__NR_move_pages];
	ksyms.old_sys_perf_event_open =
		(void *)sys_call_table[__NR_perf_event_open];
	ksyms.old_sys_clone =
		(void *)sys_call_table[__NR_clone];
	ksyms.old_sys_fork =
		(void *)sys_call_table[__NR_fork];
	ksyms.old_sys_vfork =
		(void *)sys_call_table[__NR_vfork];
	ksyms.old_sys_reboot =
		(void *)sys_call_table[__NR_reboot];

	/* kernel data */
	ksyms.die_chain = (void *)COMPILE_TIME_DIE_CHAIN;

	/* kernel API */
	ksyms.on_each_cpu = on_each_cpu;
	ksyms._copy_from_user = _copy_from_user;
	ksyms._copy_to_user = _copy_to_user;
	ksyms.register_die_notifier = register_die_notifier;
	ksyms.unregister_die_notifier = unregister_die_notifier;
	ksyms.filp_open = (void *)filp_open;
	ksyms.filp_close = (void *)filp_close;

	return 0;
}
