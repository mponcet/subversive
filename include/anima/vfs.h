#ifndef __VFS_H
#define __VFS_H

typedef int (*__filldir_t)(void *, const char *, int, loff_t, u64, unsigned);
struct __dir_context {
	__filldir_t actor;
	loff_t pos;
};

void vfs_debug(void);
void vfs_hook(void);
int vfs_hide_filename(const char *name, unsigned int len);
int vfs_unhide_filename(const char *name, unsigned int len);

#endif
