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
LIBAIO_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libaio/build/usr)
LIBFUSE_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libfuse/build)
LIBNCURSES_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libncurses/build)
LIBNUMA_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libnuma/build)
LIBUUID_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libuuid/build)
CUNIT_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/cunit/build)
OPENSSL_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/openssl/build)
SPDK_CFLAGS  += "-I$(LIBFUSE_DIR)/include -I$(OPENSSL_DIR)/include -I$(LIBUUID_DIR)/include -I$(LIBAIO_DIR)/include -I$(CUNIT_DIR)/include -I$(LIBNCURSES_DIR)/include -I$(LIBNUMA_DIR)/include"
SPDK_LDFLAGS += "-L$(LIBFUSE_DIR)/lib -L$(OPENSSL_DIR)/lib -L$(LIBUUID_DIR)/lib -L$(LIBAIO_DIR)/lib -L$(CUNIT_DIR)/lib -L$(LIBNCURSES_DIR)/lib -L$(LIBNUMA_DIR)/lib"

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