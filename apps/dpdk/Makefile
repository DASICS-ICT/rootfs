NAME    = dpdk
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(DASICS_SUPPORT)/repo/include
LDFLAGS += -L$(DASICS_SUPPORT)/build -lDASICS -T$(DASICS_SUPPORT)/repo/ld.lds
BUILD_NAME = riscv64-build-gcc
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

DPDK_REPO   = $(abspath repo)
BUILD_DIR   = $(DPDK_REPO)/$(BUILD_NAME)
MESON_OPTIONS += \
	--cross-file config/riscv/riscv64_linux_gcc \
	-Dexamples=all \
	-Denable_dbchecker=true
	 

.PHONY: dpdk

dpdk:
	cd $(DPDK_REPO) && \
	meson setup --prefix=$(DPDK_REPO)/build $(BUILD_NAME) $(MESON_OPTIONS)
	cd $(DPDK_REPO) && \
	ninja -C $(BUILD_NAME) && \
	ninja -C $(BUILD_NAME) install

$(APP): dpdk
# 	ln -sf $(BUILD_DIR)/app/dpdk-testpmd $(RISCV_ROOTFS_HOME)/rootfsimg/bin/dpdk-testpmd
	ln -sf $(BUILD_DIR)/examples/dpdk-simpleperf $(RISCV_ROOTFS_HOME)/rootfsimg/bin/dpdk-simpleperf
# 	ln -sf $(BUILD_DIR)/examples/dpdk-l2fwd $(RISCV_ROOTFS_HOME)/rootfsimg/bin/dpdk-l2fwd