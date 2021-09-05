#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_DR_PROTECT  1 /* debug register access protection */
#define CONFIG_PATCH_DEBUG 0 /* 0 => use die_notifier, 1 => patch_debug */

/* rootkit interface */
#define MAGIC_NUMBER_GET_ROOT 0x42424242 /* TODO: should be randomized */
#define MAGIC_NUMBER_DEBUG_RK 0x43434343

/* VFS */
#define FILE_HIDDEN_PREFIX "hidden_"

#endif
