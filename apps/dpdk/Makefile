NAME    = dpdk-testpmd
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
TESTPMD_ELF    = $(DPDK_REPO)/riscv64-build-gcc/app/dpdk-testpmd
SIMPLEPERF_ELF = $(DPDK_REPO)/riscv64-build-gcc/examples/dpdk-simpleperf
BUILD_DIR   = $(DPDK_REPO)/riscv64-build-gcc
MESON_OPTIONS += \
	-Dexamples=all --cross-file config/riscv/riscv64_linux_gcc 

.PHONY: dpdk

dpdk:
	cd $(DPDK_REPO) && \
	meson setup --prefix=$(DPDK_REPO)/build riscv64-build-gcc $(MESON_OPTIONS)
	cd $(DPDK_REPO) && \
	ninja -C riscv64-build-gcc && \
	ninja -C riscv64-build-gcc install

$(APP): dpdk
	ln -sf $(TESTPMD_ELF) $@
	ln -sf $(SIMPLEPERF_ELF) $(RISCV_ROOTFS_HOME)/rootfsimg/bin/dpdk-simpleperf
