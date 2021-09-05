#ifndef __VFS_H
#define __VFS_H

#include <linux/kernel.h>

typedef int (*__filldir_t)(void *, const char *, int, loff_t, u64, unsigned);
struct __dir_context {
	__filldir_t actor;
	loff_t pos;
};

void vfs_hook(void);

#endif
