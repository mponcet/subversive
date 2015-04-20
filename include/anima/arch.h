#ifndef __ARCH_H
#define __ARCH_H

#ifdef ARCH_ARM

#undef ARCH_ARM
#define ARCH_ARM 1
#define ARCH_X86 0
#include <anima/arm.h>

#elif ARCH_X86

#undef ARCH_X86
#define ARCH_X86 1
#define ARCH_ARM 0
#include <anima/x86.h>

#endif

#endif
