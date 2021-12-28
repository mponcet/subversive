#include <linux/kdebug.h>
#include <linux/kernel.h>

#include <subversive/config.h>
#include <subversive/ksyms.h>
#include <subversive/x86.h>

static struct dr_breakpoint bps;

static void
noinline get_dr(unsigned char num, unsigned long *val)
{
	switch (num) {
	case 0:
		__get_dr(0, *val);
		break;
	case 1:
		__get_dr(1, *val);
		break;
	case 2:
		__get_dr(2, *val);
		break;
	case 3:
		__get_dr(3, *val);
		break;
	case 6:
		__get_dr(6, *val);
		break;
	case 7:
		__get_dr(7, *val);
		break;
	}
}

static void
noinline set_dr(unsigned char num, unsigned long val)
{
	switch (num) {
	case 0:
		__set_dr(0, val);
		break;
	case 1:
		__set_dr(1, val);
		break;
	case 2:
		__set_dr(2, val);
		break;
	case 3:
		__set_dr(3, val);
		break;
	case 6:
		__set_dr(6, val);
		break;
	case 7:
		__set_dr(7, val);
		break;
	}
}

static inline void __on_each_cpu_set_dr(void *data)
{
	unsigned long *dr = data;
	set_dr(dr[0], dr[1]);
}

static inline void on_each_cpu_set_dr(unsigned char num, unsigned long val)
{
	unsigned long dr[2] = { num, val };
	ksyms.on_each_cpu(__on_each_cpu_set_dr, dr, 1);
}

static void emulate_mov_db(int access_ok, unsigned char op,
			   unsigned int dr, unsigned long *reg)
{
	pr_debug("%s: op=%s access_ok=%d\n",
			__func__, op == 0x23 ? "set" : "get", access_ok);

	if (op == 0x23) { /* mov reg,drX */
		if (access_ok) {
			set_dr(dr, *reg);
		} else {
			/* prevent overwriting already set debug registers */
			if (dr >= 0 && dr <= 3 && !bps.dr[dr])
				set_dr(dr, *reg);
			else if (dr == 6)
				set_dr(dr, *reg);
			else if (dr == 7)
				;
		}
	} else { /* mov drX,reg */
		if (access_ok) {
			get_dr(dr, reg);
		} else {
			if (dr >= 0 && dr <= 3)
				*reg = bps.old_dr[dr];
			else if (dr == 6)
				*reg = bps.old_dr6;
			else if (dr == 7)
				*reg = bps.old_dr7;
		}
	}
}

static int emulate_cpu(struct pt_regs *regs)
{
	int access_ok = 0;
	/* op3 is x86_64 specific : mov dbX,r9-15 */
	unsigned char op0, op1, op2, op3;

	op0 = *(unsigned char *)regs->ip;
	op1 = *(unsigned char *)(regs->ip + 1);
	op2 = *(unsigned char *)(regs->ip + 2);
	op3 = *(unsigned char *)(regs->ip + 3);

	pr_debug("%s: op0=%x op1=%x op2=%x op3=%x\n",
			__func__, op0, op1, op2, op3);

	/*
	 * set_dr and get_dr are the only function
	 * allowed to access debug registers
	 */
	if (regs->ip >= (unsigned long)set_dr
	    && regs->ip < ((unsigned long)set_dr + 256))
		access_ok = 1;
	else if (regs->ip >= (unsigned long)get_dr
	    && regs->ip < ((unsigned long)get_dr + 256))
		access_ok = 1;

	if (op0 == 0x0f && (op1 == 0x23 || op1 == 0x21)) {
		switch (op2) {
		/* db0 handling */
		case 0xc0:
			emulate_mov_db(access_ok, op1, 0, &regs->ax);
			break;
		case 0xc3:
			emulate_mov_db(access_ok, op1, 0, &regs->bx);
			break;
		case 0xc1:
			emulate_mov_db(access_ok, op1, 0, &regs->cx);
			break;
		case 0xc2:
			emulate_mov_db(access_ok, op1, 0, &regs->dx);
			break;
		case 0xc7:
			emulate_mov_db(access_ok, op1, 0, &regs->di);
			break;
		case 0xc6:
			emulate_mov_db(access_ok, op1, 0, &regs->si);
			break;
		case 0xc4:
			emulate_mov_db(access_ok, op1, 0, &regs->sp);
			break;
		case 0xc5:
			emulate_mov_db(access_ok, op1, 0, &regs->bp);
			break;

		/* db1 handling */
		case 0xc8:
			emulate_mov_db(access_ok, op1, 1, &regs->ax);
			break;
		case 0xcb:
			emulate_mov_db(access_ok, op1, 1, &regs->bx);
			break;
		case 0xc9:
			emulate_mov_db(access_ok, op1, 1, &regs->cx);
			break;
		case 0xca:
			emulate_mov_db(access_ok, op1, 1, &regs->dx);
			break;
		case 0xcf:
			emulate_mov_db(access_ok, op1, 1, &regs->di);
			break;
		case 0xce:
			emulate_mov_db(access_ok, op1, 1, &regs->si);
			break;
		case 0xcc:
			emulate_mov_db(access_ok, op1, 1, &regs->sp);
			break;
		case 0xcd:
			emulate_mov_db(access_ok, op1, 1, &regs->bp);
			break;

		/* db2 handling */
		case 0xd0:
			emulate_mov_db(access_ok, op1, 2, &regs->ax);
			break;
		case 0xd3:
			emulate_mov_db(access_ok, op1, 2, &regs->bx);
			break;
		case 0xd1:
			emulate_mov_db(access_ok, op1, 2, &regs->cx);
			break;
		case 0xd2:
			emulate_mov_db(access_ok, op1, 2, &regs->dx);
			break;
		case 0xd7:
			emulate_mov_db(access_ok, op1, 2, &regs->di);
			break;
		case 0xd6:
			emulate_mov_db(access_ok, op1, 2, &regs->si);
			break;
		case 0xd4:
			emulate_mov_db(access_ok, op1, 2, &regs->sp);
			break;
		case 0xd5:
			emulate_mov_db(access_ok, op1, 2, &regs->bp);
			break;

		/* db3 handling */
		case 0xd8:
			emulate_mov_db(access_ok, op1, 3, &regs->ax);
			break;
		case 0xdb:
			emulate_mov_db(access_ok, op1, 3, &regs->bx);
			break;
		case 0xd9:
			emulate_mov_db(access_ok, op1, 3, &regs->cx);
			break;
		case 0xda:
			emulate_mov_db(access_ok, op1, 3, &regs->dx);
			break;
		case 0xdf:
			emulate_mov_db(access_ok, op1, 3, &regs->di);
			break;
		case 0xde:
			emulate_mov_db(access_ok, op1, 3, &regs->si);
			break;
		case 0xdc:
			emulate_mov_db(access_ok, op1, 3, &regs->sp);
			break;
		case 0xdd:
			emulate_mov_db(access_ok, op1, 3, &regs->bp);
			break;

		/* db6 handling */
		case 0xf0:
			emulate_mov_db(access_ok, op1, 6, &regs->ax);
			break;
		case 0xf3:
			emulate_mov_db(access_ok, op1, 6, &regs->bx);
			break;
		case 0xf1:
			emulate_mov_db(access_ok, op1, 6, &regs->cx);
			break;
		case 0xf2:
			emulate_mov_db(access_ok, op1, 6, &regs->dx);
			break;
		case 0xf7:
			emulate_mov_db(access_ok, op1, 6, &regs->di);
			break;
		case 0xf6:
			emulate_mov_db(access_ok, op1, 6, &regs->si);
			break;
		case 0xf4:
			emulate_mov_db(access_ok, op1, 6, &regs->sp);
			break;
		case 0xf5:
			emulate_mov_db(access_ok, op1, 7, &regs->bp);
			break;

		/* db7 handling */
		case 0xf8:
			emulate_mov_db(access_ok, op1, 7, &regs->ax);
			break;
		case 0xfb:
			emulate_mov_db(access_ok, op1, 7, &regs->bx);
			break;
		case 0xf9:
			emulate_mov_db(access_ok, op1, 7, &regs->cx);
			break;
		case 0xfa:
			emulate_mov_db(access_ok, op1, 7, &regs->dx);
			break;
		case 0xff:
			emulate_mov_db(access_ok, op1, 7, &regs->di);
			break;
		case 0xfe:
			emulate_mov_db(access_ok, op1, 7, &regs->si);
			break;
		case 0xfc:
			emulate_mov_db(access_ok, op1, 7, &regs->sp);
			break;
		case 0xfd:
			emulate_mov_db(access_ok, op1, 7, &regs->bp);
			break;
		}
		regs->ip += 3;
	} else if (op0 == 0x41 && op1 == 0x0f && (op2 == 0x21 || op2 == 0x23)) {
		switch (op3) {
		/* db0 handling */
		case 0xc0:
			emulate_mov_db(access_ok, op2, 0, &regs->r8);
			break;
		case 0xc1:
			emulate_mov_db(access_ok, op2, 0, &regs->r9);
			break;
		case 0xc2:
			emulate_mov_db(access_ok, op2, 0, &regs->r10);
			break;
		case 0xc3:
			emulate_mov_db(access_ok, op2, 0, &regs->r11);
			break;
		case 0xc4:
			emulate_mov_db(access_ok, op2, 0, &regs->r12);
			break;
		case 0xc5:
			emulate_mov_db(access_ok, op2, 0, &regs->r13);
			break;
		case 0xc6:
			emulate_mov_db(access_ok, op2, 0, &regs->r14);
			break;
		case 0xc7:
			emulate_mov_db(access_ok, op2, 0, &regs->r15);
			break;
		/* db1 handling */
		case 0xc8:
			emulate_mov_db(access_ok, op2, 1, &regs->r8);
			break;
		case 0xc9:
			emulate_mov_db(access_ok, op2, 1, &regs->r9);
			break;
		case 0xca:
			emulate_mov_db(access_ok, op2, 1, &regs->r10);
			break;
		case 0xcb:
			emulate_mov_db(access_ok, op2, 1, &regs->r11);
			break;
		case 0xcc:
			emulate_mov_db(access_ok, op2, 1, &regs->r12);
			break;
		case 0xcd:
			emulate_mov_db(access_ok, op2, 1, &regs->r13);
			break;
		case 0xce:
			emulate_mov_db(access_ok, op2, 1, &regs->r14);
			break;
		case 0xcf:
			emulate_mov_db(access_ok, op2, 1, &regs->r15);
			break;
		/* db2 handling */
		case 0xd0:
			emulate_mov_db(access_ok, op2, 2, &regs->r8);
			break;
		case 0xd1:
			emulate_mov_db(access_ok, op2, 2, &regs->r9);
			break;
		case 0xd2:
			emulate_mov_db(access_ok, op2, 2, &regs->r10);
			break;
		case 0xd3:
			emulate_mov_db(access_ok, op2, 2, &regs->r11);
			break;
		case 0xd4:
			emulate_mov_db(access_ok, op2, 2, &regs->r12);
			break;
		case 0xd5:
			emulate_mov_db(access_ok, op2, 2, &regs->r13);
			break;
		case 0xd6:
			emulate_mov_db(access_ok, op2, 2, &regs->r14);
			break;
		case 0xd7:
			emulate_mov_db(access_ok, op2, 2, &regs->r15);
			break;
		/* db3 handling */
		case 0xd8:
			emulate_mov_db(access_ok, op2, 3, &regs->r8);
			break;
		case 0xd9:
			emulate_mov_db(access_ok, op2, 3, &regs->r9);
			break;
		case 0xda:
			emulate_mov_db(access_ok, op2, 3, &regs->r10);
			break;
		case 0xdb:
			emulate_mov_db(access_ok, op2, 3, &regs->r11);
			break;
		case 0xdc:
			emulate_mov_db(access_ok, op2, 3, &regs->r12);
			break;
		case 0xdd:
			emulate_mov_db(access_ok, op2, 3, &regs->r13);
			break;
		case 0xde:
			emulate_mov_db(access_ok, op2, 3, &regs->r14);
			break;
		case 0xdf:
			emulate_mov_db(access_ok, op2, 3, &regs->r15);
			break;
		/* db6 handling */
		case 0xf0:
			emulate_mov_db(access_ok, op2, 6, &regs->r8);
			break;
		case 0xf1:
			emulate_mov_db(access_ok, op2, 6, &regs->r9);
			break;
		case 0xf2:
			emulate_mov_db(access_ok, op2, 6, &regs->r10);
			break;
		case 0xf3:
			emulate_mov_db(access_ok, op2, 6, &regs->r11);
			break;
		case 0xf4:
			emulate_mov_db(access_ok, op2, 6, &regs->r12);
			break;
		case 0xf5:
			emulate_mov_db(access_ok, op2, 6, &regs->r13);
			break;
		case 0xf6:
			emulate_mov_db(access_ok, op2, 6, &regs->r14);
			break;
		case 0xf7:
			emulate_mov_db(access_ok, op2, 6, &regs->r15);
			break;
		/* db7 handling */
		case 0xf8:
			emulate_mov_db(access_ok, op2, 7, &regs->r8);
			break;
		case 0xf9:
			emulate_mov_db(access_ok, op2, 7, &regs->r9);
			break;
		case 0xfa:
			emulate_mov_db(access_ok, op2, 7, &regs->r10);
			break;
		case 0xfb:
			emulate_mov_db(access_ok, op2, 7, &regs->r11);
			break;
		case 0xfc:
			emulate_mov_db(access_ok, op2, 7, &regs->r12);
			break;
		case 0xfd:
			emulate_mov_db(access_ok, op2, 7, &regs->r13);
			break;
		case 0xfe:
			emulate_mov_db(access_ok, op2, 7, &regs->r14);
			break;
		case 0xff:
			emulate_mov_db(access_ok, op2, 7, &regs->r15);
			break;
		}
		regs->ip += 4;
	} else {
		pr_debug("%s: unknown opcode\n", __func__);
		return 1;
	}

	return 0;
}

static int hw_breakpoint_handler(struct pt_regs *regs, unsigned long dr6)
{
	if (dr6 & DR6_BD) {
		pr_debug("%s: DR6_BD ip=%lx\n", __func__, regs->ip);
		emulate_cpu(regs);
		return 0;
	}

	for (int i = 0; i < 4; i++) {
		if ((dr6 & (DR6_TRAP0 << i)) && bps.handlers[i]) {
			bps.handlers[i](regs);
			/* FIXME: RF only if exec breakpoint */
			regs->flags |= X86_EFLAGS_RF;
			return 0;
		}
	}

	/* breakpoint not handled */
	return 1;
}

static int hw_breakpoint_notify(struct notifier_block *self, unsigned long val, void *data)
{
	int ret;
	unsigned long dr6;
	struct die_args *args = (struct die_args *)data;
	struct pt_regs *regs = args->regs;

	if (val != DIE_DEBUG)
		return NOTIFY_DONE;

	/* clear dr7 to prevent debug exceptions */
	set_dr(7, 0);

	/* DR7_GD disabled : use __get_dr and __set_dr */
	__set_dr(6, (unsigned long)DR6_RESERVED);

	dr6 = *(unsigned long *)args->err;
	ret = hw_breakpoint_handler(regs, dr6);
	ret = ret ? NOTIFY_DONE : NOTIFY_STOP;

	__set_dr(7, bps.dr7);

	return ret;
}

static void new_do_debug(struct pt_regs *regs, long error_code)
{
	int ret;
	unsigned long dr6;

	/* clear dr7 to prevent debug exceptions */
	set_dr(7, 0);

	/* DR7_GD disabled : use __get_dr and __set_dr */
	__get_dr(6, dr6);
	__set_dr(6, (unsigned long)DR6_RESERVED);

	ret = hw_breakpoint_handler(regs, dr6);
	if (ret)
		ksyms.old_do_debug(regs, error_code);

	__set_dr(7, bps.dr7);
}

static unsigned int *patched_addr = NULL;
static unsigned int old_rip_off;

static int patch_debug_entry(void)
{
	int call_found = 0;
	unsigned int rip_offset;
	unsigned char *ptr = (unsigned char *)get_idt_handler(1);

	for (int i = 0; i < 128; i++) {
		if (call_found == 2) {
			patched_addr = (unsigned int *)ptr;
			old_rip_off = *patched_addr;
			rip_offset = (unsigned long)new_do_debug -
				     (unsigned long)patched_addr - 4;
			ksyms.old_do_debug = (void *)(old_rip_off +
					     (unsigned long)patched_addr) + 4;

			cr0_wp_disable();
			*patched_addr = rip_offset;
			cr0_wp_enable();
			return 0;

		} else if (ptr[0] == 0xe8) {
			call_found++;
		}
		ptr++;
	}

	return -1;
}

static void restore_debug_entry(void)
{
	cr0_wp_disable();
	*patched_addr = old_rip_off;
	cr0_wp_enable();
}


static int debug_handler_patched;
static struct notifier_block hw_breakpoint_notifier_block = {
	.notifier_call = hw_breakpoint_notify,
	.priority = INT_MAX,
};

/*
 * exported functions
 */
void x86_hw_breakpoint_debug(void)
{
	unsigned long dr;

	pr_debug("%s: debug registers state\n", __func__);
	for (int i = 0; i <= 7; i++) {
		if (i == 4 || i == 5)
			continue;
		get_dr(i, &dr);
		pr_debug("\tdr%d=%lx\n", i, dr);
	}

	/* retry with debug register protection */
	pr_debug("%s: debug registers state (dr protect)\n", __func__);
	__get_dr(0, dr);
	pr_debug("\tdr0=%lx\n", dr);
	__get_dr(1, dr);
	pr_debug("\tdr1=%lx\n", dr);
	__get_dr(2, dr);
	pr_debug("\tdr2=%lx\n", dr);
	__get_dr(3, dr);
	pr_debug("\tdr3=%lx\n", dr);
	__get_dr(6, dr);
	pr_debug("\tdr6=%lx\n", dr);
	__get_dr(7, dr);
	pr_debug("\tdr7=%lx\n", dr);

	pr_debug("%s: trying to set debug registers (dr protect)\n", __func__);
	dr=0xbadc0ded;
	pr_debug("\tdr0\n");
	__set_dr(0, dr);
	pr_debug("\tdr1\n");
	__set_dr(1, dr);
	pr_debug("\tdr2\n");
	__set_dr(2, dr);
	pr_debug("\tdr3\n");
	__set_dr(3, dr);
	pr_debug("\tdr6\n");
	__set_dr(6, dr);
	pr_debug("\tdr7\n");
	__set_dr(7, dr);
}

#include <linux/notifier.h>
int x86_hw_breakpoint_init(void)
{
	memset(&bps, 0, sizeof(bps));

	/* save debug registers, current cpu */
	for (int i = 0; i < 4; i++)
		get_dr(i, &bps.old_dr[i]);
	get_dr(6, &bps.old_dr6);
	get_dr(7, &bps.old_dr7);

	pr_debug("%s: dr0=%lx dr1=%lx dr2=%lx dr3=%lx dr6=%lx dr7=%lx\n",
		 __func__, bps.old_dr[0], bps.old_dr[1], bps.old_dr[2],
		 bps.old_dr[3], bps.old_dr6, bps.old_dr7);

	if (CONFIG_PATCH_DEBUG || !ksyms.die_chain || !ksyms.register_die_notifier) {
		pr_debug("%s: patching debug handler\n", __func__);
		debug_handler_patched = !patch_debug_entry();
		if (!debug_handler_patched)
			return -1;
	} else {
		/*
		 * We want to be called first in the call chain.
		 * Since hw_breakpoint_exceptions_nb->priority == INT_MAX
		 * (see kernel/events/hw_breakpoint.c), lower it before
		 * adding our own handler.
		 */
		struct atomic_notifier_head *anh = (struct atomic_notifier_head *)ksyms.die_chain;
		struct notifier_block *h = anh->head;
		h->priority--;
		ksyms.register_die_notifier(&hw_breakpoint_notifier_block);
		h->priority++;
		pr_debug("%s: registering die notifier\n", __func__);
	}

	return 0;
}

int x86_hw_breakpoint_exit(void)
{
	/* restore saved debug registers */
	on_each_cpu_set_dr(7, bps.old_dr7);
	on_each_cpu_set_dr(6, bps.old_dr6);
	for (int i = 0; i < 4; i++)
		on_each_cpu_set_dr(i, bps.old_dr[i]);

	if (CONFIG_PATCH_DEBUG) {
		if (debug_handler_patched)
			restore_debug_entry();
	} else if (ksyms.unregister_die_notifier) {
		ksyms.unregister_die_notifier(&hw_breakpoint_notifier_block);
	}

	return 0;
}

/*
 * x86_hw_breakpoint_register: enable debug register breakpoint
 */
int x86_hw_breakpoint_register(int dr_nr, unsigned long addr, int type,
				int len, bp_handler handler)
{
	unsigned long dr7 = 0;

	if (dr_nr >= 4 || dr_nr < 0)
		return -1;

	bps.dr[dr_nr] = addr;
	bps.handlers[dr_nr] = handler;

	dr7 = (len << 2) | type;
	dr7 <<= (16 + dr_nr * 4);	/* len and type */
	dr7 |= 0x2 << (dr_nr * 2);	/* global breakpoint */
	bps.dr7 |= bps.dr7 | dr7 | DR7_GE;
	pr_debug("%s: dr%d=0x%lx dr7=%lx\n", __func__, dr_nr, addr, bps.dr7);
	on_each_cpu_set_dr(dr_nr, bps.dr[dr_nr]);
	on_each_cpu_set_dr(7, bps.dr7);

	return 0;
}

/*
 * x86_hw_breakpoint_unregister: disable debug register in dr7, restore old register
 */
int x86_hw_breakpoint_unregister(int dr_nr)
{
	if (dr_nr >= 4)
		return -1;

	bps.dr[dr_nr] = 0;
	/* disable global breakpoint */
	bps.dr7 &= ~(0x2 << dr_nr * 2);
	pr_debug("%s: dr%d=0x%lx dr7=%lx\n", __func__, dr_nr, bps.dr[dr_nr], bps.dr7);
	on_each_cpu_set_dr(dr_nr, bps.dr[dr_nr]);
	on_each_cpu_set_dr(7, bps.dr7);

	return 0;
}

/*
 * x86_hw_breakpoint_protect_enable: enable debug registers protection
 */
void x86_hw_breakpoint_protect_enable(void)
{
	pr_debug("%s: enable DR7_GD\n", __func__);
	bps.dr7 |= DR7_GD;
	on_each_cpu_set_dr(7, bps.dr7);
}

void x86_hw_breakpoint_protect_disable(void)
{
	pr_debug("%s: disable DR7_GD\n", __func__);
	bps.dr7 &= ~DR7_GD;
	on_each_cpu_set_dr(7, bps.dr7);
}
