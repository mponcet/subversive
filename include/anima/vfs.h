#ifndef __VFS_H
#define __VFS_H

typedef int (*__filldir_t)(void *, const char *, int, loff_t, u64, unsigned);
struct __dir_context {
	__filldir_t actor;
	loff_t pos;
};

void hook_vfs(void);
int hide_filename(const char *name, unsigned int len);
int unhide_filename(const char *name, unsigned int len);

#endif
