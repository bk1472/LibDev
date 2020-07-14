#cpu				?= x
cpu				?= a
PWD				:= $(shell pwd)
SRC_DIR			:= $(PWD)



ifeq ($(cpu), a)
CPU				:= aarch64
RT				?= -rt49
KDIR			:= /opt/tx2-kernel-header/$(CPU)-linux-4.4$(RT)-header-tx2_3.2
ARCH			:= arm64
CROSS_COMPILE	:= aarch64-linux-gnu-
BUILD_OPT		:= ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
EXT_FLAGS		:= -D__AARCH64__
else
CPU				:= x64
KDIR			:=	/lib/modules/$(shell uname -r)/build
EXT_FLAGS		:= -D__X86_64__
endif
OUT_DIR			 = ./out/$(CPU)

TARGET			 = lgdriver

obj-m			:= $(TARGET).o


SRCS			 =
SRCS			+= lgdrv_main.c
SRCS			+= lgdrv_mapinfo.c
SRCS			+= lgdrv_sp.c

$(TARGET)-objs	:= $(foreach src, $(SRCS), $(src:.c=.o))

ccflags-y		+= -I. -I./include $(EXT_FLAGS)

all:
	@$(MAKE) -C $(KDIR) M=$(SRC_DIR) $(BUILD_OPT) modules
	@test -d $(OUT_DIR) || mkdir -p $(OUT_DIR)
	@rm -rf $(OUT_DIR)/.tmp_versions
	@mv *.o  *.ko .*.cmd Module.* modules.* .tmp_versions  $(OUT_DIR)
	@rm -f *.mod.c

clean:
	@rm -rf $(OUT_DIR)

clobber:
	@rm -rf ./out
