#include <asm/msr.h>
#include <asm/msr-index.h>

#include <subversive/arch.h>

unsigned long system_call;
unsigned long sys_call_table;
unsigned long sys_call_table_call;

static unsigned long ia32_sys_call_table;
static unsigned long ia32_sys_call_table_call;

void find_sys_call_table(void)
{
	int i;

        rdmsrl(MSR_LSTAR, system_call);

	unsigned char *op = (unsigned char *)system_call;
	for (i = 0; i < 512; i++) {
		if (is_call_sct(op)) {
			sys_call_table = *(unsigned int *)&op[3] | ~0xffffffffUL;
			sys_call_table_call = (unsigned long)op;
			break;
		}
		op++;
	}
}

/*
 * 32bits syscall emulation
 */
void find_ia32_sys_call_table(void)
{
	int i;
	unsigned char *op = (unsigned char *)get_idt_handler(0x80);

	system_call = (unsigned long)op;

	for (i = 0; i < 512; i++) {
		if (is_call_sct(op)) {
			ia32_sys_call_table = *(unsigned int *)&op[3] | ~0xffffffffUL;
			ia32_sys_call_table_call = (unsigned long)op;
			return;
		}
		op++;
	}
}
