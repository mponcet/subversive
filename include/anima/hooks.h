#ifndef __HOOK_H
#define __HOOK_H

#define NR_SYSCALLS 320 /* for x86_64 */
int hook_sys_call_table(void);

#endif
