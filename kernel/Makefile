obj-m += subversive.o
subversive-objs := init.o arch.o dr_breakpoint.o hook.o

EXTRA_CFLAGS = -Wall -I$(PWD)/../include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
