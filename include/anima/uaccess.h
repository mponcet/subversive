#ifndef __UACCESS_H
#define __UACCESS_H

#include <asm/uaccess.h>

/* access kernel space */
static mm_segment_t old_fs;
#define SET_KERNEL_DS do { old_fs = get_fs(); set_fs(KERNEL_DS); } while (0)
#define SET_OLD_FS do { set_fs(old_fs); } while (0)

#endif
