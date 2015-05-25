#include <anima/arch.h>
#include <anima/debug.h>
#include <anima/vfs.h>

#ifdef DEBUG

void debug_rk(void)
{
	arch_hw_breakpoint_debug();
	vfs_debug();
}

#else

void debug_rk(void)
{
}

#endif
