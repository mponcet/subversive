#ifndef __X86_H
#define __X86_H

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

static inline unsigned long __read_cr0(void)
{
	unsigned long cr0;
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	return cr0;
}

static inline void __write_cr0(unsigned long cr0)
{
	asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

static inline void cr0_wp_enable(void)
{
	unsigned long cr0;

	cr0 = __read_cr0() | 0x10000;
	__write_cr0(cr0);
	asm volatile("sti");
}

static inline void cr0_wp_disable(void)
{
	unsigned long cr0;

	asm volatile("cli");
	cr0 = __read_cr0() & ~0x10000;
	__write_cr0(cr0);
}

struct pt_regs;
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
#define DR6_TRAP0 (1 << 0)
#define DR6_TRAP1 (1 << 1)
#define DR6_TRAP2 (1 << 2)
#define DR6_TRAP3 (1 << 3)
#define DR6_BD	 (1 << 13)
#define DR6_BS	 (1 << 14)
#define DR6_BT	 (1 << 15)
#define DR6_RESERVED 0xffff0ff0


/*
 * DR7
 */
#define DR7_LE (1 << 8)
#define DR7_GE (1 << 9)
#define DR7_GD (1 << 13)

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

struct __debugreg {
	unsigned char num;
	unsigned long val;
};

void x86_hw_breakpoint_debug(void);
int x86_hw_breakpoint_init(void);
int x86_hw_breakpoint_exit(void);
int x86_hw_breakpoint_register(int dr, unsigned long addr, int type, int len, bp_handler handler);
int x86_hw_breakpoint_unregister(int dr);
void x86_hw_breakpoint_protect_enable(void);
void x86_hw_breakpoint_protect_disable(void);

#endif
