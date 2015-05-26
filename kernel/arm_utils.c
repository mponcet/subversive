#include <linux/kernel.h>
#include <linux/kdebug.h>

#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/arm.h>

static unsigned long arm_get_die_chain_addr(void)
{
	/* TODO: stub */
	return 0;
}

static int __get_sycall_addrs(unsigned long base,
				 unsigned long *sys_call_table,
				 unsigned long *sys_call_table_call)
{
	unsigned long *op = (unsigned long *)base;

	for (int i = 0; i < 512; i++, op++) {
		if ((*op & 0xfffff000) == 0xe28f8000) {
			unsigned offset = (*op & 0xfff) + 8;
			*sys_call_table = (unsigned long)op + offset;
			*sys_call_table_call = (unsigned long)op;
			return 0;
		}
	}

	return -1;
}

/*
 * arm_get_kernel_syms: get syscall table addr
 */
int arm_get_kernel_syms(void)
{
	int ret;

	ksyms.vector_swi = get_symbol_addr("vector_swi");
	if (!ksyms.vector_swi)
		return -1;

	ret = __get_sycall_addrs(ksyms.vector_swi, &ksyms.sys_call_table,
			   &ksyms.sys_call_table_call);

	ksyms.die_chain = (void *)arm_get_die_chain_addr();

	pr_debug("%s: vector_swi=%lx sys_call_table=%lx sys_call_table_call=%lx\n",
		 __func__, ksyms.vector_swi,
		ksyms.sys_call_table, ksyms.sys_call_table_call);

	pr_debug("%s: die_chain=%p\n", __func__, ksyms.die_chain);

	return ret;
}
