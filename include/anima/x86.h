#ifndef __X86_H
#define __X86_H

#include <linux/smp.h>

struct idtr {
        unsigned short limit;
        unsigned long base;
} __attribute__((packed));

struct idt_desc {
	unsigned short off0_15;
        unsigned short sel;
        unsigned char none, flags;
        unsigned short off16_31;
        unsigned int off32_63;
	unsigned int reserved;
} __attribute__((packed));

static inline void sidt(struct idtr *idtr)
{
	asm volatile("sidt %0" : "=m" (*idtr));
}

static inline struct idt_desc *get_idt_entry_addr(int off)
{
	struct idtr idtr;

	sidt(&idtr);

	return (struct idt_desc *)idtr.base + off;
}

static inline unsigned long get_idt_handler(int off)
{
	struct idt_desc *desc = get_idt_entry_addr(off);

	return desc->off0_15
		| ((unsigned long)desc->off16_31 << 16)
		| ((unsigned long)desc->off32_63 << 32);
}

static inline void set_CR0_WP(void)
{
	unsigned long cr0;

	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x00010000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

static inline void clear_CR0_WP(void)
{
	unsigned long cr0;

	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 &= ~0x00010000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

int x86_get_kernel_syms(void);

typedef void (*bp_handler)(struct pt_regs *regs);

struct dr_breakpoint {
	unsigned long dr[4];
	unsigned long dr6, dr7;
	bp_handler handlers[4];
	unsigned long old_dr[4];
	unsigned long old_dr6, old_dr7;
};

enum bp_type {
	BP_EXEC = 0,
	BP_RW,
};

/*
 * DR6
 */
#define DR_TRAP0 (1 << 0)
#define DR_TRAP1 (1 << 1)
#define DR_TRAP2 (1 << 2)
#define DR_TRAP3 (1 << 3)
#define DR_BD	 (1 << 13)
#define DR_BS	 (1 << 14)
#define DR_BT	 (1 << 15)


/*
 * DR7
 */
#define DR_LE (1 << 8)
#define DR_GE (1 << 9)
#define DR_GD (1 << 13)

#define DR_RW_EXECUTE 0x0
#define DR_RW_WRITE   0x1
#define DR_RW_READ    0x3

#define DR_LEN_1 0x0
#define DR_LEN_2 0x1
#define DR_LEN_4 0x3
#define DR_LEN_8 0x2

#define __set_dr(num, val) \
	asm volatile ("mov %0,%%db" #num : : "r" (val))
#define __get_dr(num, val) \
        asm volatile("mov %%db" #num ",%0" : "=r" (val))

static inline void get_dr(unsigned char num, unsigned long *val)
{
	switch (num) {
	case 0:
		__get_dr(0, *val);
		break;
	case 1:
		__get_dr(1, *val);
		break;
	case 2:
		__get_dr(2, *val);
		break;
	case 3:
		__get_dr(3, *val);
		break;
	case 6:
		__get_dr(6, *val);
		break;
	case 7:
		__get_dr(7, *val);
		break;
	}
}

static inline void set_dr(unsigned char num, unsigned long val)
{
	switch (num) {
	case 0:
		__set_dr(0, val);
		break;
	case 1:
		__set_dr(1, val);
		break;
	case 2:
		__set_dr(2, val);
		break;
	case 3:
		__set_dr(3, val);
		break;
	case 6:
		__set_dr(6, val);
		break;
	case 7:
		__set_dr(7, val);
		break;
	}
}

struct __debugreg {
	unsigned char num;
	unsigned long val;
};

static inline void __on_each_cpu_set_dr(void *data)
{
	struct __debugreg *dr = data;
	set_dr(dr->num, dr->val);
}

static inline void on_each_cpu_set_dr(unsigned char num, unsigned long val)
{
	struct __debugreg dr = {
		.num = num,
		.val = val,
	};

	on_each_cpu(__on_each_cpu_set_dr, &dr, 1);
}

int x86_hw_breakpoint_init(void);
int x86_hw_breakpoint_exit(void);
int x86_hw_breakpoint_register(int dr, unsigned long addr, int type, int len, bp_handler handler);
int x86_hw_breakpoint_unregister(int dr);

#endif
