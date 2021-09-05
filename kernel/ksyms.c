#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>

#include <subversive/ksyms.h>

struct kernel_syms ksyms;

struct ksym_lookup_struct {
	const char *name;
	unsigned long addr;
};

static int ksym_lookup(void *data, const char *name,
			struct module *module, unsigned long addr)
{
	struct ksym_lookup_struct *kls = data;

	kls->addr = 0;
	if (!strcmp(kls->name, name)) {
		pr_debug("%s: %s=%lx\n", __func__, name, addr);
		kls->addr = addr;
		return 1;
	}

	return 0;
}

unsigned long get_symbol_addr(const char *name)
{
	struct ksym_lookup_struct kls;

	kls.name = name;
	kallsyms_on_each_symbol(ksym_lookup, &kls);

	return kls.addr;
}

/* find die_chain */
static unsigned long x86_get_die_chain_addr(void)
{
	unsigned long die_chain = 0;
	unsigned char *ptr;

	ptr = (unsigned char *)get_symbol_addr("unregister_die_notifier");
	if (!ptr)
		goto exit;

	for (int i = 0; i < 32; i++) {
		if (ptr[i] == 0x48 && ptr[i+1] == 0xc7 && ptr[i+2] == 0xc7) {
			/* mov $die_chain,%rdi */
			die_chain = *(unsigned int *)&ptr[i+3] | ~0xffffffffUL;
			break;
		}
	}

exit:
	return die_chain;
}



int get_kernel_syms(void)
{
	ksyms.do_syscall_64 = get_symbol_addr("do_syscall_64");
	ksyms.sys_call_table = get_symbol_addr("sys_call_table");

	ksyms.die_chain = x86_get_die_chain_addr();

	/* kernel API */
	ksyms.on_each_cpu = (void *)get_symbol_addr("on_each_cpu");
	ksyms.register_die_notifier = (void *)get_symbol_addr("register_die_notifier");
	ksyms.unregister_die_notifier = (void *)get_symbol_addr("unregister_die_notifier");
	ksyms.commit_creds = (void *)get_symbol_addr("commit_creds");
	ksyms.prepare_kernel_cred = (void *)get_symbol_addr("prepare_kernel_cred");

	return 0;
}
