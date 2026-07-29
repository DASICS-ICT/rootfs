# Common out-of-tree Kbuild wrapper for DASICS regression modules.
#
# The including Makefile must define MODULE_NAME and its obj-m/composite
# objects. PEER_MODULE is optional and names a sibling module whose
# Module.symvers is required by the trusted companion.

ifeq ($(KERNELRELEASE),)

RISCV_ROOTFS_HOME ?= $(abspath $(MODULE_DIR)/../../..)
KERNEL_SRC ?= $(abspath $(RISCV_ROOTFS_HOME)/../riscv-linux)
ARCH ?= riscv
CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
PROFILE ?= runtime-regression
MODULE_SET := $(if $(filter runtime-regression,$(PROFILE)),runtime,$(PROFILE))
MODULE_INSTALL_DIR ?= $(abspath \
	$(RISCV_ROOTFS_HOME)/rootfsimg/root/modules/dasics/$(MODULE_SET))
KBUILD_ARGS := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

ifneq ($(PEER_MODULE),)
PEER_MODULE_DIR := $(abspath \
	$(RISCV_ROOTFS_HOME)/apps/dasics-test/$(PEER_MODULE))
KBUILD_ARGS += KBUILD_EXTRA_SYMBOLS=$(PEER_MODULE_DIR)/Module.symvers
endif

.PHONY: all install clean

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(MODULE_DIR) $(KBUILD_ARGS) modules

install: all
	mkdir -p $(MODULE_INSTALL_DIR)
	cp $(MODULE_DIR)/$(MODULE_NAME).ko $(MODULE_INSTALL_DIR)/

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(MODULE_DIR) $(KBUILD_ARGS) clean
	rm -f $(MODULE_INSTALL_DIR)/$(MODULE_NAME).ko

endif
