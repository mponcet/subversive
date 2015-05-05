#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>

#include <anima/arch.h>
#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/libc.h>

struct kernel_syms ksyms;

struct ksym_lookup_struct {
	const char *name;
	unsigned long addr;
};

static int ksym_lookup(void *data, const char *name,
			struct module *module, unsigned long addr)
{
	struct ksym_lookup_struct *kls = data;

	kls->addr = 0;
	if (!anima_strcmp(kls->name, name)) {
		pr_debug("%s: %s=%lx\n", __func__, name, addr);
		kls->addr = addr;
		return 1;
	}

	return 0;
}

unsigned long get_symbol_addr(const char *name)
{
	struct ksym_lookup_struct kls;

	kls.name = name;
	kallsyms_on_each_symbol(ksym_lookup, &kls);

	return kls.addr;
}

int get_kernel_syms(void)
{
	unsigned long *sys_call_table;

#if ARCH_X86
	if (x86_get_kernel_syms())
		return -1;
#elif ARCH_ARM
	if (arm_get_kernel_syms())
		return -1;
#endif

	sys_call_table = (unsigned long *)ksyms.sys_call_table;

	ksyms.old_sys_stat =
		(void *)sys_call_table[__NR_stat];
	ksyms.old_sys_lstat =
		(void *)sys_call_table[__NR_lstat];
#if ARCH_X86
	ksyms.old_sys_fstatat =
		(void *)sys_call_table[__NR_newfstatat];
#endif
	ksyms.old_sys_chdir =
		(void *)sys_call_table[__NR_chdir];
	ksyms.old_sys_open =
		(void *)sys_call_table[__NR_open];
	ksyms.old_sys_getdents =
		(void *)sys_call_table[__NR_getdents];
	ksyms.old_sys_getdents64 =
		(void *)sys_call_table[__NR_getdents64];
	ksyms.old_sys_execve =
		(void *)sys_call_table[__NR_execve];
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
#if ARCH_X86
	ksyms.old_sys_migrate_pages =
		(void *)sys_call_table[__NR_migrate_pages];
#endif
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
	ksyms.on_each_cpu = (void *)get_symbol_addr("on_each_cpu");
	ksyms._copy_from_user = (void *)get_symbol_addr("_copy_from_user");
	ksyms._copy_to_user = (void *)get_symbol_addr("_copy_to_user");
	ksyms.register_die_notifier =
		(void *)get_symbol_addr("register_die_notifier");
	ksyms.unregister_die_notifier =
		(void *)get_symbol_addr("unregister_die_notifier");
	ksyms.register_keyboard_notifier =
		(void *)get_symbol_addr("register_keyboard_notifier");
	ksyms.unregister_keyboard_notifier =
		(void *)get_symbol_addr("unregister_keyboard_notifier");
	ksyms.filp_open = (void *)get_symbol_addr("filp_open");
	ksyms.filp_close = (void *)get_symbol_addr("filp_close");
	ksyms.vmalloc = (void *)get_symbol_addr("vmalloc");
	ksyms.vfree = (void *)get_symbol_addr("vfree");

	/* kernel libc */
	ksyms.strncmp = (void *)get_symbol_addr("strcmp");
	ksyms.strlcat = (void *)get_symbol_addr("strlcat");
	ksyms.snprintf = (void *)get_symbol_addr("snprintf");

	return 0;
}
