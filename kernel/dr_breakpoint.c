#include <linux/kernel.h>

#include <subversive/dr_breakpoint.h>
#include <subversive/hook.h>
#include <subversive/arch.h>

struct dr_breakpoint bp;

static void emulate_cpu(struct pt_regs *regs)
{
	regs->ip += 3;
}

asmlinkage void my_do_debug(struct pt_regs *regs, long error_code)
{
	unsigned long dr6;
	bp_handler handler;

	get_dr(6, &dr6);

	if (dr6 & DR_BD) {
		dr6 &= ~DR_BD;
		emulate_cpu(regs);
	}
	if (dr6 & DR_TRAP0) {
		dr6 &= ~DR_TRAP0;
		handler = bp.handlers[0];
		goto trap;
	} else if (dr6 & DR_TRAP1) {
		dr6 &= ~DR_TRAP1;
		handler = bp.handlers[1];
		goto trap;
	} else if (dr6 & DR_TRAP2) {
		dr6 &= ~DR_TRAP2;
		handler = bp.handlers[2];
		goto trap;
	} else if (dr6 & DR_TRAP3) {
		dr6 &= ~DR_TRAP3;
		handler = bp.handlers[3];
		goto trap;
	}
	return;
trap:
	regs->flags |= X86_EFLAGS_RF;
	regs->flags &= ~X86_EFLAGS_TF;
	handler(regs);
}

static unsigned int *patched_addr = NULL;
static unsigned int old_rip_off;
/*
 * patch the call to do_debug inside debug interrupt handler
 */
int patch_do_debug(void)
{
	int i, call_found = 0;
	unsigned int rip_offset;
	unsigned char *ptr = (unsigned char *)get_idt_handler(1);

	for (i = 0; i < 128; i++) {
		if (call_found == 2) {
			patched_addr = (unsigned int *)ptr;
			old_rip_off = *patched_addr;
			rip_offset = (unsigned long)my_do_debug - (unsigned long)patched_addr - 4;

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

void restore_do_debug(void)
{
	clear_CR0_WP();
	*patched_addr = old_rip_off;
	set_CR0_WP();
}

int register_dr_breakpoint(unsigned long addr, enum bp_type type, bp_handler handler)
{
	int i;

	for (i = 0; i < 4; i++)
		if (!bp.dr[i])
			break;

	if (i == 4)
		return -1;
	
	bp.dr[i] = addr;
	bp.handlers[i] = handler;

	switch (i) {
	case 0:
		bp.dr7 |= DR0_TRAP_GLOBAL;
		bp.dr7 &= ~(DR_RW_EXECUTE << DR0_RW_OFF);
		bp.dr7 &= ~(0 << DR0_LEN_OFF);
		break;
	case 1:
		bp.dr7 |= DR1_TRAP_GLOBAL;
		bp.dr7 |= DR_RW_EXECUTE << DR1_RW_OFF;
		bp.dr7 |= 0 << DR1_LEN_OFF;
		break;
	case 2:
		bp.dr7 |= DR2_TRAP_GLOBAL;
		bp.dr7 |= DR_RW_EXECUTE << DR2_RW_OFF;
		bp.dr7 |= 0 << DR2_LEN_OFF;
		break;
	case 3:
		bp.dr7 |= DR3_TRAP_GLOBAL;
		bp.dr7 |= DR_RW_EXECUTE << DR3_RW_OFF;
		bp.dr7 |= 0 << DR3_LEN_OFF;
		break;
	}
	/* bp.dr7 |= DR_GD; */
	bp.dr7 |= DR_LE | DR_GE;
	on_each_cpu_set_dr(i, bp.dr[i]);
	on_each_cpu_set_dr(7, bp.dr7);

	return 0;
}

int unregister_dr_bp(unsigned long addr)
{
	int i;

	for (i = 0; i < 4; i++)
		if (bp.dr[i] == addr)
			break;

	if (i == 4)
		return -1;

	switch (i) {
	case 0:
		bp.dr7 &= ~DR0_TRAP_GLOBAL;
		break;
	case 1:
		bp.dr7 &= ~DR1_TRAP_GLOBAL;
		break;
	case 2:
		bp.dr7 &= ~DR2_TRAP_GLOBAL;
		break;
	case 3:
		bp.dr7 &= ~DR3_TRAP_GLOBAL;
		break;
	}
	on_each_cpu_set_dr(i, bp.dr[i]);
	on_each_cpu_set_dr(7, bp.dr7);

	return 0;
}
