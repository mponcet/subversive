#include <linux/module.h>
#include <linux/kernel.h>

#include <subversive/hook.h>
#include <subversive/dr_breakpoint.h>

static unsigned long old_dr[4];
static unsigned long old_dr6, old_dr7;
static int do_debug_patched;

MODULE_LICENSE("GPL");

static int __init subversive_init(void)
{
	int i;

	for (i = 0; i < 4; i++)
		get_dr(i, &old_dr[i]);

	get_dr(6, &old_dr6);
	get_dr(7, &old_dr7);

	do_debug_patched = !patch_do_debug();
	if (!do_debug_patched)
		return 1;

	hook_sys_call_table();

	return 0;
}

static void __exit subversive_exit(void)
{
	int i;

	if (do_debug_patched)
		restore_do_debug();

	for (i = 0; i < 4; i++)
		on_each_cpu_set_dr(i, old_dr[i]);

	on_each_cpu_set_dr(6, old_dr6);
	on_each_cpu_set_dr(7, old_dr7);
}

module_init(subversive_init);
module_exit(subversive_exit);
