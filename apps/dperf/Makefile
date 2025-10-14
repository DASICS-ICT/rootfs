NAME    = dperf
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
DPDK_PATH = $(abspath $(RISCV_ROOTFS_HOME)/apps/dpdk/repo)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

DPERF_REPO   = $(abspath repo)
DPERF_ELF    = $(DPERF_REPO)/build/dperf
DPDK_PKGCONFIG_DIR  = $(abspath $(DPDK_PATH))/build/lib/pkgconfig

.PHONY: dperf

dperf:
	cd $(DPERF_REPO) && make PKG_CONFIG_PATH=$(DPDK_PKGCONFIG_DIR) CC=$(CROSS_COMPILE)-gcc

$(APP): dperf
	ln -sf $(DPERF_ELF) $@
