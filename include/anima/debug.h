#ifndef __DEBUG_H
#define __DEBUG_H

#include <linux/kernel.h>

struct syscall_stat {
	unsigned long nr_call;
};

#ifdef DEBUG

extern struct syscall_stat sys_stats[];
#define SYS_STATS_INC(sys) sys_stats[__NR_##sys].nr_call++
static inline void sys_stats_init(int sys)
{
	sys_stats[sys].nr_call = 1;
}

#else

#define SYS_STATS_INC(sys)
static inline void sys_stats_init(int sys)
{
}

#endif

void debug_sys_stats(void);
void debug_rk(void);

#endif
