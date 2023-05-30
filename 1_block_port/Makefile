KERNDIR=/lib/modules/$(shell uname -r)/build
KERNEL_HEADERS := /usr/src/linux-headers-5.15.0-72-generic

obj-m+=packet_filter.o
objs+=packet_filter.o
PWD=$(shell pwd)

default:
	make -C $(KERNDIR) M=$(PWD) modules

clean:
	make -C $(KERNDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.o
