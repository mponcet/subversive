#include <linux/kernel.h>

#include <anima/ksyms.h>

#define READ_WB_REG_CASE(OP2, M, VAL)			\
	case ((OP2 << 4) + M):				\
		ARM_DBG_READ(c0, c ## M, OP2, VAL);	\
		break

#define WRITE_WB_REG_CASE(OP2, M, VAL)			\
	case ((OP2 << 4) + M):				\
		ARM_DBG_WRITE(c0, c ## M, OP2, VAL);	\
		break

#define GEN_READ_WB_REG_CASES(OP2, VAL)		\
	READ_WB_REG_CASE(OP2, 0, VAL);		\
	READ_WB_REG_CASE(OP2, 1, VAL);		\
	READ_WB_REG_CASE(OP2, 2, VAL);		\
	READ_WB_REG_CASE(OP2, 3, VAL);		\
	READ_WB_REG_CASE(OP2, 4, VAL);		\
	READ_WB_REG_CASE(OP2, 5, VAL);		\
	READ_WB_REG_CASE(OP2, 6, VAL);		\
	READ_WB_REG_CASE(OP2, 7, VAL);		\
	READ_WB_REG_CASE(OP2, 8, VAL);		\
	READ_WB_REG_CASE(OP2, 9, VAL);		\
	READ_WB_REG_CASE(OP2, 10, VAL);		\
	READ_WB_REG_CASE(OP2, 11, VAL);		\
	READ_WB_REG_CASE(OP2, 12, VAL);		\
	READ_WB_REG_CASE(OP2, 13, VAL);		\
	READ_WB_REG_CASE(OP2, 14, VAL);		\
	READ_WB_REG_CASE(OP2, 15, VAL)

#define GEN_WRITE_WB_REG_CASES(OP2, VAL)	\
	WRITE_WB_REG_CASE(OP2, 0, VAL);		\
	WRITE_WB_REG_CASE(OP2, 1, VAL);		\
	WRITE_WB_REG_CASE(OP2, 2, VAL);		\
	WRITE_WB_REG_CASE(OP2, 3, VAL);		\
	WRITE_WB_REG_CASE(OP2, 4, VAL);		\
	WRITE_WB_REG_CASE(OP2, 5, VAL);		\
	WRITE_WB_REG_CASE(OP2, 6, VAL);		\
	WRITE_WB_REG_CASE(OP2, 7, VAL);		\
	WRITE_WB_REG_CASE(OP2, 8, VAL);		\
	WRITE_WB_REG_CASE(OP2, 9, VAL);		\
	WRITE_WB_REG_CASE(OP2, 10, VAL);	\
	WRITE_WB_REG_CASE(OP2, 11, VAL);	\
	WRITE_WB_REG_CASE(OP2, 12, VAL);	\
	WRITE_WB_REG_CASE(OP2, 13, VAL);	\
	WRITE_WB_REG_CASE(OP2, 14, VAL);	\
	WRITE_WB_REG_CASE(OP2, 15, VAL)

static u32 read_wb_reg(int n)
{
	u32 val = 0;

	switch (n) {
	GEN_READ_WB_REG_CASES(ARM_OP2_BVR, val);
	GEN_READ_WB_REG_CASES(ARM_OP2_BCR, val);
	GEN_READ_WB_REG_CASES(ARM_OP2_WVR, val);
	GEN_READ_WB_REG_CASES(ARM_OP2_WCR, val);
	default:
		pr_warn("attempt to read from unknown breakpoint register %d\n",
			n);
	}

	return val;
}

static void write_wb_reg(int n, u32 val)
{
	switch (n) {
	GEN_WRITE_WB_REG_CASES(ARM_OP2_BVR, val);
	GEN_WRITE_WB_REG_CASES(ARM_OP2_BCR, val);
	GEN_WRITE_WB_REG_CASES(ARM_OP2_WVR, val);
	GEN_WRITE_WB_REG_CASES(ARM_OP2_WCR, val);
	default:
		pr_warn("attempt to write to unknown breakpoint register %d\n",
			n);
	}
	asm volatile("isb" : : : "memory");
}

static int
hw_breakpoint_notify(struct notifier_block *self, unsigned long val, void *data)
{
	pr_debug("%s\n", __func__);

	return NOTIFY_STOP;
}

#include <linux/notifier.h>
static struct notifier_block hw_breakpoint_notifier_block = {
        .notifier_call = hw_breakpoint_notify,
        .priority = INT_MAX,
};

/* Determine number of BRP registers available. */
static int get_num_brp_resources(void)
{
	u32 didr;
	ARM_DBG_READ(c0, c0, 0, didr);
	return ((didr >> 24) & 0xf) + 1;
}

static int monitor_mode_enabled(void)
{
	u32 dscr;
	ARM_DBG_READ(c0, c1, 0, dscr);
	return !!(dscr & ARM_DSCR_MDBGEN);
}

void arm_hw_breakpoint_debug(void)
{
	for (int i = 0; i < 4; i++)
		pr_debug("%s: debug register %d == %u\n", __func__, i, read_wb_reg(ARM_BASE_BVR + i));

	pr_debug("%s: number of breakpoints == %d\n", __func__, get_num_brp_resources());
	pr_debug("%s: monitor mode == %d\n", __func__, monitor_mode_enabled());
}

int arm_hw_breakpoint_init(void)
{
	ksyms.register_die_notifier(&hw_breakpoint_notifier_block);
	return 0;
}

int arm_hw_breakpoint_exit(void)
{
	ksyms.unregister_die_notifier(&hw_breakpoint_notifier_block);
	return 0;
}

/*
 * arm_hw_breakpoint_register: exec breakpoint
 * */
int arm_hw_breakpoint_register(unsigned long addr)
{
	u32 mismatch = 0;
	u32 len = ARM_BREAKPOINT_LEN_4;
	u32 type = ARM_BREAKPOINT_EXECUTE;
	u32 privilege = ARM_BREAKPOINT_PRIV;
	u32 enabled = 1;
	u32 ctrl = (mismatch << 22) | (len << 5) | (type << 3) |
		   (privilege << 1) | enabled;

	/* write address register */
	write_wb_reg(ARM_BASE_BVR + 0, addr);
	/* write control register */
	write_wb_reg(ARM_BASE_BCR + 0, ctrl);
	return 0;
}

int arm_hw_breakpoint_unregister(unsigned long addr)
{
	;
}
