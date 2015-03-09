#ifndef __VFS_H
#define __VFS_H

#ifdef TMP_INCLUDE_DIRCTX
typedef int (*filldir_t)(void *, const char *, int, loff_t, u64, unsigned);
struct dir_context {
	filldir_t actor;
	loff_t pos;
};
#endif

void hook_vfs(void);
int hide_filename(const char *name, unsigned int len);
int unhide_filename(const char *name, unsigned int len);

#endif
