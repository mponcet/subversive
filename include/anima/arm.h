#ifndef __ARM_H
#define __ARM_H

#include <asm/cacheflush.h>
#include <anima/libc.h>

static inline void arm_write_hook(void *addr, char *data, unsigned int size)
{
	anima_memcpy(addr, data, size);
	flush_icache_range((unsigned long)addr, (unsigned long)addr + size);
}

int arm_get_kernel_syms(void);

#endif
