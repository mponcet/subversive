#ifndef __KSYMS_H
#define __KSYMS_H

#include <linux/kdebug.h>
#include <linux/notifier.h>

struct kernel_syms {
	/* syscalls */
	unsigned long sys_call_table;
	unsigned long do_syscall_64;

	/* kernel functions */
	void (*old_do_debug)(struct pt_regs *, long);

	/* kernel data */
	unsigned long die_chain;

	/* kernel API */
	void (*on_each_cpu)(void (*)(void *), void *, int);
	int (*register_die_notifier)(struct notifier_block *);
	int (*unregister_die_notifier)(struct notifier_block *);
	int (*commit_creds)(void *);
	void *(*prepare_kernel_cred)(void *);
};

extern struct kernel_syms ksyms;

unsigned long get_symbol_addr(const char *name);
int get_kernel_syms(void);

#endif
