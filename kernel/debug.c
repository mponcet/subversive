#include <subversive/arch.h>
#include <subversive/debug.h>
#include <subversive/vfs.h>

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
