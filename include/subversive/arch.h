#ifndef __ARCH_H
#define __ARCH_H

#ifdef ARCH_ARM

#undef ARCH_ARM
#define ARCH_ARM 1
#define ARCH_X86 0
#include <subversive/arm.h>

#define arch_hw_breakpoint_init arm_hw_breakpoint_init
#define arch_hw_breakpoint_exit	arm_hw_breakpoint_exit
#define arch_hw_breakpoint_debug arm_hw_breakpoint_debug
#define arch_get_kernel_syms arm_get_kernel_syms

#elif ARCH_X86

#undef ARCH_X86
#define ARCH_X86 1
#define ARCH_ARM 0
#include <subversive/x86.h>

#define arch_hw_breakpoint_init x86_hw_breakpoint_init
#define arch_hw_breakpoint_exit x86_hw_breakpoint_exit
#define arch_hw_breakpoint_debug x86_hw_breakpoint_debug
#define arch_get_kernel_syms x86_get_kernel_syms

#endif

#endif
