#ifndef __ARCH_H
#define __ARCH_H

/*
 * amd64 architecture declarations
 */

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

/*
 * opcodes of call *sys_call_table(,%rax,8)
 */

static inline int is_call_sct(unsigned char *op)
{
	if (op[0] == 0xff && op[1] == 0x14 && op[2] == 0xc5)
		return 1;

	return 0;
}

void find_sys_call_table(void);
void find_ia32_sys_call_table(void);

extern unsigned long system_call;
extern unsigned long sys_call_table;
extern unsigned long sys_call_table_call;

#endif
