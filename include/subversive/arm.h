#ifndef __ARM_H
#define __ARM_H

#include <asm/cacheflush.h>
#include <subversive/libc.h>

/* Debug architecture numbers. */
#define ARM_DEBUG_ARCH_RESERVED 0       /* In case of ptrace ABI updates. */
#define ARM_DEBUG_ARCH_V6       1
#define ARM_DEBUG_ARCH_V6_1     2
#define ARM_DEBUG_ARCH_V7_ECP14 3
#define ARM_DEBUG_ARCH_V7_MM    4
#define ARM_DEBUG_ARCH_V7_1     5
#define ARM_DEBUG_ARCH_V8       6

/* Breakpoint */
#define ARM_BREAKPOINT_EXECUTE  0

/* Watchpoints */
#define ARM_BREAKPOINT_LOAD     1
#define ARM_BREAKPOINT_STORE    2
#define ARM_FSR_ACCESS_MASK     (1 << 11)

/* Privilege Levels */
#define ARM_BREAKPOINT_PRIV     1
#define ARM_BREAKPOINT_USER     2

/* Lengths */
#define ARM_BREAKPOINT_LEN_1    0x1
#define ARM_BREAKPOINT_LEN_2    0x3
#define ARM_BREAKPOINT_LEN_4    0xf
#define ARM_BREAKPOINT_LEN_8    0xff

/* Limits */
#define ARM_MAX_BRP             16
#define ARM_MAX_WRP             16
#define ARM_MAX_HBP_SLOTS       (ARM_MAX_BRP + ARM_MAX_WRP)

/* DSCR method of entry bits. */
#define ARM_DSCR_MOE(x)                 ((x >> 2) & 0xf)
#define ARM_ENTRY_BREAKPOINT            0x1
#define ARM_ENTRY_ASYNC_WATCHPOINT      0x2
#define ARM_ENTRY_SYNC_WATCHPOINT       0xa

/* DSCR monitor/halting bits. */
#define ARM_DSCR_HDBGEN         (1 << 14)
#define ARM_DSCR_MDBGEN         (1 << 15)

/* OSLSR os lock model bits */
#define ARM_OSLSR_OSLM0         (1 << 0)

/* opcode2 numbers for the co-processor instructions. */
#define ARM_OP2_BVR             4
#define ARM_OP2_BCR             5
#define ARM_OP2_WVR             6
#define ARM_OP2_WCR             7

/* Base register numbers for the debug registers. */
#define ARM_BASE_BVR            64
#define ARM_BASE_BCR            80
#define ARM_BASE_WVR            96
#define ARM_BASE_WCR            112

/* Accessor macros for the debug registers. */
#define ARM_DBG_READ(N, M, OP2, VAL) do {\
        asm volatile("mrc p14, 0, %0, " #N "," #M ", " #OP2 : "=r" (VAL));\
} while (0)

#define ARM_DBG_WRITE(N, M, OP2, VAL) do {\
        asm volatile("mcr p14, 0, %0, " #N "," #M ", " #OP2 : : "r" (VAL));\
} while (0)

static inline void arm_write_hook(void *addr, char *data, unsigned int size)
{
	subversive_memcpy(addr, data, size);
	flush_icache_range((unsigned long)addr, (unsigned long)addr + size);
}

int arm_get_kernel_syms(void);

void arm_hw_breakpoint_debug(void);
int arm_hw_breakpoint_register(unsigned long addr);
int arm_hw_breakpoint_init(void);
int arm_hw_breakpoint_exit(void);

/* Inline hooking API */
int arm_inline_breakpoint_register(void *target, void *new);
int arm_inline_breakpoint_unregister(void *target);

#endif
