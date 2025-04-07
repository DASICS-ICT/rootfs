NAME = tmod
obj-m += $(NAME).o
TARGET := $(NAME).ko
KERNEL_SRC ?= ~/dasics/riscv-linux
PWD := $(shell pwd)
ARCH ?= riscv
CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
ROOTFSIMG := ~/dasics/rootfs/rootfsimg/root
BUILD_DIR := $(PWD)/build
FLAGS ?= -fPIC -g
ccflags-y += $(FLAGS)
KBUILD_EXTRA_SYMBOLS := ~/dasics/rootfs/apps/dasics-test/utmod/Module.symvers

install:
	mkdir $(BUILD_DIR)
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	cp $(TARGET) $(BUILD_DIR)
	ln -sf $(BUILD_DIR)/$(TARGET) $(ROOTFSIMG)/$(TARGET)
	

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -rf $(BUILD_DIR)
