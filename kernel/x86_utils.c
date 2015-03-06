#include <linux/kernel.h>
#include <linux/kdebug.h>

#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/x86.h>

/*
 * find {ia32_}sys_call_table addr
 * find "call {ia32_}sys_call_table(,%rax,8)" addr
 */
static int __get_sycall_addrs(unsigned long base,
				 unsigned long *sys_call_table,
				 unsigned long *sys_call_table_call)
{
	unsigned char *op;

	op = (unsigned char *)base;
	for (int i = 0; i < 512; i++) {
		/*
		 * call *sys_call_table(,%rax,8) opcodes
		 */
		if (op[0] == 0xff && op[1] == 0x14 && op[2] == 0xc5) {
			*sys_call_table = (unsigned long)((*(u32 *)&op[3]) | ~0xffffffffUL);
			*sys_call_table_call = (unsigned long)op;
			return 0;
		}
		op++;
	}

	return -1;
}

/*
 * x86_get_kernel_syms: get syscall table addr
 * ia32 addrs are not mandatory (CONFIG_IA32_EMULATION=n ?)
 */
int x86_get_kernel_syms(void)
{
	int ret = 0;

	/*
	 * ia32 emulation via sysenter (not used ?)
	 */
	rdmsrl(MSR_IA32_SYSENTER_EIP, ksyms.ia32_sysenter);
	if (ksyms.ia32_sysenter) {
		ret = __get_sycall_addrs(ksyms.ia32_sysenter,
					 &ksyms.ia32_sys_call_table,
					 &ksyms.ia32_sysenter_sys_call_table_call);
		if (ret)
			goto exit;
	}

	/*
	 * ia32 emulation via syscall
	 */
	ksyms.ia32_syscall = get_idt_handler(0x80);
	if (ksyms.ia32_syscall) {
		ret = __get_sycall_addrs(ksyms.ia32_syscall,
					  &ksyms.ia32_sys_call_table,
					  &ksyms.ia32_syscall_sys_call_table_call);
		if (ret)
			goto exit;
	}

	/*
	 * x86_64
	 */
	rdmsrl(MSR_LSTAR, ksyms.system_call);
	if (ksyms.system_call)
		ret = __get_sycall_addrs(ksyms.system_call,
					  &ksyms.sys_call_table,
					  &ksyms.sys_call_table_call);
	else
		ret = -1;

exit:
	pr_debug("%s: ia32_sysenter=%lx ia32_sysenter_sys_call_table_call=%lx\n",
		 __func__, ksyms.ia32_sysenter, ksyms.ia32_sysenter_sys_call_table_call);
	pr_debug("%s: ia32_syscall=%lx ia32_syscall_sys_call_table_call=%lx\n",
		 __func__, ksyms.ia32_syscall, ksyms.ia32_syscall_sys_call_table_call);
	pr_debug("%s: ia32_sys_call_table=%lx\n", __func__, ksyms.ia32_sys_call_table);

	return ret;
}
