NAME    = spdk
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
BUILD_NAME = riscv64-build-gcc
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu
LIBFUSE_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libfuse/repo/build)
SPDK_CFLAGS  += -I$(LIBFUSE_DIR)/include
SPDK_LDFLAGS += -L$(LIBFUSE_DIR)/lib

DPDK_REPO   = $(abspath repo)/dpdk
SPDK_REPO   = $(abspath repo)
BUILD_DIR   = $(SPDK_REPO)/$(BUILD_NAME)

MESON_OPTIONS += \
	--cross-file config/riscv/riscv64_linux_gcc \
	-Dexamples=all \
# 	-Denable_dbchecker=true
	 
.PHONY: spdk

dpdk:
	cd $(DPDK_REPO) && \
	meson setup --prefix=$(DPDK_REPO)/build $(BUILD_NAME) $(MESON_OPTIONS)
	cd $(DPDK_REPO) && \
	ninja -C $(BUILD_NAME) && \
	ninja -C $(BUILD_NAME) install

spdk: dpdk
	cd $(SPDK_REPO) && \
	CC=$(CROSS_COMPILE)-gcc CXX=$(CROSS_COMPILE)-g++ CFLAGS=$(SPDK_CFLAGS) LDFLAGS=$(SPDK_LDFLAGS) ./configure --target-arch=rv64gc --cross-prefix=$(CROSS_COMPILE) --with-dpdk=$(DPDK_REPO)/build  
	

$(APP): spdk