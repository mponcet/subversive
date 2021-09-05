obj-m += subversive.o
subversive-objs := init.o ksyms.o syscalls.o vfs.o x86_hw_breakpoint.o

EXTRA_CFLAGS = -std=gnu99 -I$(PWD)/../include

ifeq ($(DEBUG), 1)
	EXTRA_CFLAGS += -DDEBUG
endif

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" clean
