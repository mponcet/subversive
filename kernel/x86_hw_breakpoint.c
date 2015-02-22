#include <linux/kdebug.h>
#include <linux/kernel.h>

#include <anima/config.h>
#include <anima/debug.h>
#include <anima/ksyms.h>
#include <anima/x86.h>

static struct dr_breakpoint bps;

static void emulate_mov_db(unsigned char op, unsigned int dr, unsigned long *reg)
{
	/*
	 * FIXME : get_dr mostly done, but should handle other kernel path trying to
	 * set dr
	 */
	if (op == 0x23) { /* mov reg,drX */
		if (rk_cfg.state == RK_SHUTDOWN || rk_cfg.state == RK_BOOT) {
			set_dr(dr, *reg);
		} else {
			switch (dr) {
			case 0:
			case 1:
			case 2:
			case 3:
				if (!bps.dr[dr])
					set_dr(dr, *reg);
				break;
			case 6:
				set_dr(6, *reg);
				break;
			case 7:
				set_dr(7, *reg);
				break;
			}
		}
	} else { /* mov drX,reg */
		if (rk_cfg.state == RK_SHUTDOWN || rk_cfg.state == RK_BOOT) {
			get_dr(dr, reg);
		} else {
			switch (dr) {
			case 0:
			case 1:
			case 2:
			case 3:
				*reg = bps.old_dr[dr];
				break;
			case 6:
				*reg = bps.old_dr6;
				break;
			case 7:
				*reg = bps.old_dr7;
				break;
			}
		}
	}
}

static int emulate_cpu(struct pt_regs *regs)
{
	/* op3 is x86_64 specific : mov dbX,r9-15 */
	unsigned char op0, op1, op2, op3;

	op0 = *(unsigned char *)regs->ip;
	op1 = *(unsigned char *)(regs->ip + 1);
	op2 = *(unsigned char *)(regs->ip + 2);
	op3 = *(unsigned char *)(regs->ip + 3);

	pr_debug("%s: op0=%x op1=%x op2=%x op3=%x", __func__, op0, op1, op2, op3);

	if (op0 == 0x0f && (op1 == 0x23 || op1 == 0x21)) {
		switch (op2) {
		/* db0 handling */
		case 0xc0:
			emulate_mov_db(op1, 0, &regs->ax);
			break;
		case 0xc3:
			emulate_mov_db(op1, 0, &regs->bx);
			break;
		case 0xc1:
			emulate_mov_db(op1, 0, &regs->cx);
			break;
		case 0xc2:
			emulate_mov_db(op1, 0, &regs->dx);
			break;
		case 0xc7:
			emulate_mov_db(op1, 0, &regs->di);
			break;
		case 0xc6:
			emulate_mov_db(op1, 0, &regs->si);
			break;
		case 0xc4:
			emulate_mov_db(op1, 0, &regs->sp);
			break;
		case 0xc5:
			emulate_mov_db(op1, 0, &regs->bp);
			break;

		/* db1 handling */
		case 0xc8:
			emulate_mov_db(op1, 1, &regs->ax);
			break;
		case 0xcb:
			emulate_mov_db(op1, 1, &regs->bx);
			break;
		case 0xc9:
			emulate_mov_db(op1, 1, &regs->cx);
			break;
		case 0xca:
			emulate_mov_db(op1, 1, &regs->dx);
			break;
		case 0xcf:
			emulate_mov_db(op1, 1, &regs->di);
			break;
		case 0xce:
			emulate_mov_db(op1, 1, &regs->si);
			break;
		case 0xcc:
			emulate_mov_db(op1, 1, &regs->sp);
			break;
		case 0xcd:
			emulate_mov_db(op1, 1, &regs->bp);
			break;

		/* db2 handling */
		case 0xd0:
			emulate_mov_db(op1, 2, &regs->ax);
			break;
		case 0xd3:
			emulate_mov_db(op1, 2, &regs->bx);
			break;
		case 0xd1:
			emulate_mov_db(op1, 2, &regs->cx);
			break;
		case 0xd2:
			emulate_mov_db(op1, 2, &regs->dx);
			break;
		case 0xd7:
			emulate_mov_db(op1, 2, &regs->di);
			break;
		case 0xd6:
			emulate_mov_db(op1, 2, &regs->si);
			break;
		case 0xd4:
			emulate_mov_db(op1, 2, &regs->sp);
			break;
		case 0xd5:
			emulate_mov_db(op1, 2, &regs->bp);
			break;

		/* db3 handling */
		case 0xd8:
			emulate_mov_db(op1, 3, &regs->ax);
			break;
		case 0xdb:
			emulate_mov_db(op1, 3, &regs->bx);
			break;
		case 0xd9:
			emulate_mov_db(op1, 3, &regs->cx);
			break;
		case 0xda:
			emulate_mov_db(op1, 3, &regs->dx);
			break;
		case 0xdf:
			emulate_mov_db(op1, 3, &regs->di);
			break;
		case 0xde:
			emulate_mov_db(op1, 3, &regs->si);
			break;
		case 0xdc:
			emulate_mov_db(op1, 3, &regs->sp);
			break;
		case 0xdd:
			emulate_mov_db(op1, 3, &regs->bp);
			break;

		/* db6 handling */
		case 0xf0:
			emulate_mov_db(op1, 6, &regs->ax);
			break;
		case 0xf3:
			emulate_mov_db(op1, 6, &regs->bx);
			break;
		case 0xf1:
			emulate_mov_db(op1, 6, &regs->cx);
			break;
		case 0xf2:
			emulate_mov_db(op1, 6, &regs->dx);
			break;
		case 0xf7:
			emulate_mov_db(op1, 6, &regs->di);
			break;
		case 0xf6:
			emulate_mov_db(op1, 6, &regs->si);
			break;
		case 0xf4:
			emulate_mov_db(op1, 6, &regs->sp);
			break;
		case 0xf5:
			emulate_mov_db(op1, 7, &regs->bp);
			break;

		/* db7 handling */
		case 0xf8:
			emulate_mov_db(op1, 7, &regs->ax);
			break;
		case 0xfb:
			emulate_mov_db(op1, 7, &regs->bx);
			break;
		case 0xf9:
			emulate_mov_db(op1, 7, &regs->cx);
			break;
		case 0xfa:
			emulate_mov_db(op1, 7, &regs->dx);
			break;
		case 0xff:
			emulate_mov_db(op1, 7, &regs->di);
			break;
		case 0xfe:
			emulate_mov_db(op1, 7, &regs->si);
			break;
		case 0xfc:
			emulate_mov_db(op1, 7, &regs->sp);
			break;
		case 0xfd:
			emulate_mov_db(op1, 7, &regs->bp);
			break;
		}
		regs->ip += 3;
	} else if (op0 == 0x41 && op1 == 0x0f && (op2 == 0x21 || op2 == 0x23)) {
		switch (op3) {
		/* db0 handling */
		case 0xc0:
			emulate_mov_db(op2, 0, &regs->r8);
			break;
		case 0xc1:
			emulate_mov_db(op2, 0, &regs->r9);
			break;
		case 0xc2:
			emulate_mov_db(op2, 0, &regs->r10);
			break;
		case 0xc3:
			emulate_mov_db(op2, 0, &regs->r11);
			break;
		case 0xc4:
			emulate_mov_db(op2, 0, &regs->r12);
			break;
		case 0xc5:
			emulate_mov_db(op2, 0, &regs->r13);
			break;
		case 0xc6:
			emulate_mov_db(op2, 0, &regs->r14);
			break;
		case 0xc7:
			emulate_mov_db(op2, 0, &regs->r15);
			break;
		/* db1 handling */
		case 0xc8:
			emulate_mov_db(op2, 1, &regs->r8);
			break;
		case 0xc9:
			emulate_mov_db(op2, 1, &regs->r9);
			break;
		case 0xca:
			emulate_mov_db(op2, 1, &regs->r10);
			break;
		case 0xcb:
			emulate_mov_db(op2, 1, &regs->r11);
			break;
		case 0xcc:
			emulate_mov_db(op2, 1, &regs->r12);
			break;
		case 0xcd:
			emulate_mov_db(op2, 1, &regs->r13);
			break;
		case 0xce:
			emulate_mov_db(op2, 1, &regs->r14);
			break;
		case 0xcf:
			emulate_mov_db(op2, 1, &regs->r15);
			break;
		/* db2 handling */
		case 0xd0:
			emulate_mov_db(op2, 2, &regs->r8);
			break;
		case 0xd1:
			emulate_mov_db(op2, 2, &regs->r9);
			break;
		case 0xd2:
			emulate_mov_db(op2, 2, &regs->r10);
			break;
		case 0xd3:
			emulate_mov_db(op2, 2, &regs->r11);
			break;
		case 0xd4:
			emulate_mov_db(op2, 2, &regs->r12);
			break;
		case 0xd5:
			emulate_mov_db(op2, 2, &regs->r13);
			break;
		case 0xd6:
			emulate_mov_db(op2, 2, &regs->r14);
			break;
		case 0xd7:
			emulate_mov_db(op2, 2, &regs->r15);
			break;
		/* db3 handling */
		case 0xd8:
			emulate_mov_db(op2, 3, &regs->r8);
			break;
		case 0xd9:
			emulate_mov_db(op2, 3, &regs->r9);
			break;
		case 0xda:
			emulate_mov_db(op2, 3, &regs->r10);
			break;
		case 0xdb:
			emulate_mov_db(op2, 3, &regs->r11);
			break;
		case 0xdc:
			emulate_mov_db(op2, 3, &regs->r12);
			break;
		case 0xdd:
			emulate_mov_db(op2, 3, &regs->r13);
			break;
		case 0xde:
			emulate_mov_db(op2, 3, &regs->r14);
			break;
		case 0xdf:
			emulate_mov_db(op2, 3, &regs->r15);
			break;
		/* db6 handling */
		case 0xf0:
			emulate_mov_db(op2, 6, &regs->r8);
			break;
		case 0xf1:
			emulate_mov_db(op2, 6, &regs->r9);
			break;
		case 0xf2:
			emulate_mov_db(op2, 6, &regs->r10);
			break;
		case 0xf3:
			emulate_mov_db(op2, 6, &regs->r11);
			break;
		case 0xf4:
			emulate_mov_db(op2, 6, &regs->r12);
			break;
		case 0xf5:
			emulate_mov_db(op2, 6, &regs->r13);
			break;
		case 0xf6:
			emulate_mov_db(op2, 6, &regs->r14);
			break;
		case 0xf7:
			emulate_mov_db(op2, 6, &regs->r15);
			break;
		/* db7 handling */
		case 0xf8:
			emulate_mov_db(op2, 7, &regs->r8);
			break;
		case 0xf9:
			emulate_mov_db(op2, 7, &regs->r9);
			break;
		case 0xfa:
			emulate_mov_db(op2, 7, &regs->r10);
			break;
		case 0xfb:
			emulate_mov_db(op2, 7, &regs->r11);
			break;
		case 0xfc:
			emulate_mov_db(op2, 7, &regs->r12);
			break;
		case 0xfd:
			emulate_mov_db(op2, 7, &regs->r13);
			break;
		case 0xfe:
			emulate_mov_db(op2, 7, &regs->r14);
			break;
		case 0xff:
			emulate_mov_db(op2, 7, &regs->r15);
			break;
		}
		regs->ip += 4;
	} else {
		pr_debug("%s: unknown opcode", __func__);
		return 1;
	}

	return 0;
}

static int hw_breakpoint_handler(struct pt_regs *regs, unsigned long dr6)
{
	int ret = 1;

	/* single step */
	if (dr6 & DR_BS)
		return ret;

	/* dr6 may not be cleared */
	set_dr(6, 0UL);
	/* disable breakpoints */
	set_dr(7, 0UL);

	if (dr6 & DR_BD) {
		dr6 &= ~DR_BD;
		if (rk_cfg.state == RK_SHUTDOWN && rk_cfg.dr_protect)
			bps.dr7 &= ~DR_GD;
		emulate_cpu(regs);
		ret = 0;
	}

	for (int i = 0; i < 4; i++) {
		if ((dr6 & (DR_TRAP0 << i)) && bps.handlers[i]) {
			bps.handlers[i](regs);
			/* FIXME: RF only if exec breakpoint */
			regs->flags |= X86_EFLAGS_RF;
			ret = 0;
		}
	}

	return ret;
}

static int hw_breakpoint_notify(struct notifier_block *self, unsigned long val, void *data)
{
	int ret;
	unsigned long dr6;
	struct die_args *args = (struct die_args *)data;
	struct pt_regs *regs = args->regs;

	if (val != DIE_DEBUG)
		return NOTIFY_DONE; /* FIXME: NOTIFY_OK ? */

	dr6 = *(unsigned long *)ERR_PTR(args->err);
	ret = hw_breakpoint_handler(regs, dr6);
	set_dr(7, bps.dr7);
	if (ret)
		return NOTIFY_DONE; /* FIXME: NOTIFY_OK */
	else
		return NOTIFY_STOP;
}

static void new_do_debug(struct pt_regs *regs, long error_code)
{
	int ret;
	unsigned long dr6;

	get_dr(6, &dr6);
	ret = hw_breakpoint_handler(regs, dr6);
	if (ret)
		ksyms.old_do_debug(regs, error_code);

	set_dr(7, bps.dr7);
}

static unsigned int *patched_addr = NULL;
static unsigned int old_rip_off;
/*
 * patch the call to do_debug inside debug interrupt handler
 * TODO: x86_32
 */
static int patch_debug_entry(void)
{
	int call_found = 0;
	unsigned int rip_offset;
	unsigned char *ptr = (unsigned char *)get_idt_handler(1);

	for (int i = 0; i < 128; i++) {
		if (call_found == 2) {
			patched_addr = (unsigned int *)ptr;
			old_rip_off = *patched_addr;
			rip_offset = (unsigned long)new_do_debug - (unsigned long)patched_addr - 4;
			ksyms.old_do_debug = (void *)(old_rip_off+(unsigned long)patched_addr) + 4;

			clear_CR0_WP();
			*patched_addr = rip_offset;
			set_CR0_WP();
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
	clear_CR0_WP();
	*patched_addr = old_rip_off;
	set_CR0_WP();
}


static int debug_handler_patched;
static struct notifier_block hw_breakpoint_notifier_block = {
	.notifier_call = hw_breakpoint_notify,
	.priority = INT_MAX,
};

/*
 * exported functions
 */
#include <linux/notifier.h>
int x86_hw_breakpoint_init(void)
{
	memset(&bps, 0, sizeof(bps));

	/* save debug registers, current cpu */
	for (int i = 0; i < 4; i++)
		get_dr(i, &bps.old_dr[i]);
	get_dr(6, &bps.old_dr6);
	get_dr(7, &bps.old_dr7);

	pr_debug("%s: dr0=%lx dr1=%lx dr2=%lx dr3=%lx dr6=%lx dr7=%lx",
		 __func__, bps.old_dr[0], bps.old_dr[1], bps.old_dr[2],
		 bps.old_dr[3], bps.old_dr6, bps.old_dr7);

	if (rk_cfg.patch_debug || !ksyms.die_chain) {
		rk_cfg.patch_debug = 1;
		debug_handler_patched = !patch_debug_entry();
		if (!debug_handler_patched)
			return 1;
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

	if (rk_cfg.patch_debug) {
		if (debug_handler_patched)
			restore_debug_entry();
	} else {
		ksyms.unregister_die_notifier(&hw_breakpoint_notifier_block);
	}

	return 0;
}

/*
 * register_dr_breakpoint: enable debug register breakpoint
 */
int x86_hw_breakpoint_register(int dr_nr, unsigned long addr, int type,
				int len, bp_handler handler)
{
	unsigned long dr7;

	if (dr_nr >= 4 || dr_nr < 0)
		return -1;

	/* TODO: should check if dr already used and add a force mode */

	bps.dr[dr_nr] = addr;
	bps.handlers[dr_nr] = handler;

	dr7 = (len | type) & 0xf;
	dr7 <<= (16 + dr_nr * 4);	/* len and type */
	dr7 |= 0x2 << (dr_nr * 2);	/* global breakpoint */
	bps.dr7 |= bps.dr7 | dr7 | DR_GE;
	if (rk_cfg.dr_protect)
		bps.dr7 |= DR_GD;
	pr_debug("%s: dr%d=0x%lx dr7=%lx", __func__, dr_nr, addr, bps.dr7);
	on_each_cpu_set_dr(dr_nr, bps.dr[dr_nr]);
	on_each_cpu_set_dr(7, bps.dr7);

	return 0;
}

/*
 * unregister_dr_bps: disable debug register in dr7, restore old register
 */
int x86_hw_breakpoint_unregister(int dr_nr)
{
	if (dr_nr >= 4)
		return -1;

	bps.dr[dr_nr] = 0;
	/* disable global breakpoint */
	bps.dr7 &= ~(0x2 << dr_nr * 2);
	pr_debug("%s: dr%d=0x%lx dr7=%lx", __func__, dr_nr, bps.dr[dr_nr], bps.dr7);
	on_each_cpu_set_dr(dr_nr, bps.dr[dr_nr]);
	on_each_cpu_set_dr(7, bps.dr7);

	return 0;
}
