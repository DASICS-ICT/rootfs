NAME    = dpdk-vnic
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(DASICS_SUPPORT)/repo/include
LDFLAGS += -L$(DASICS_SUPPORT)/build -lDASICS -T$(DASICS_SUPPORT)/repo/ld.lds

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

DPDK_REPO   = $(abspath repo)
#DPDK_CONFIG = $(BASH_REPO)/config.status
DPDK_ELF    = $(DPDK_REPO)/riscv64-build-gcc/examples/dpdk-vnic
BUILD_DIR   = $(DPDK_REPO)/riscv64-build-gcc
MESON_OPTIONS += \
	-Dexamples=all --cross-file config/riscv/riscv64_linux_gcc 

.PHONY: dpdk-vnic

dpdk-vnic:
	cd $(DPDK_REPO) && \
	meson setup riscv64-build-gcc $(MESON_OPTIONS)
	cd $(DPDK_REPO) && \
	ninja -C riscv64-build-gcc

$(DPDK_ELF): dpdk-vnic

$(APP): $(DPDK_ELF)
	ln -sf $(abspath $<) $@
