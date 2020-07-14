obj-m   := simple_dev.o

KDIR	:=	/lib/modules/$(shell uname -r)/build
ARCH	:=	arm64
PWD     := $(shell pwd)


all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=aarch64-linux-gnu- clean
