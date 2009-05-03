TARGET = plan9_net
OBJS = netfs.o cs.o tcp.o udp.o ether.o socklib.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += $(TARGET).o 
$(TARGET)-objs := $(OBJS)

all: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c

