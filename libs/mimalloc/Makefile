NAME    = libmimalloc.so.3
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/libmimalloc.so.3.0

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE   = riscv64-unknown-linux-gnu

MIMALLOC_REPO   = $(abspath repo)
MIMALLOC_BUILD  = $(MIMALLOC_REPO)/build
MIMALLOC_CONFIG = $(MIMALLOC_BUILD)/Makefile
MIMALLOC_ELF    = $(MIMALLOC_BUILD)/libmimalloc.so.3.0

.PHONY: libmalloc

libmalloc: $(MIMALLOC_CONFIG)
	$(MAKE) -C $(MIMALLOC_BUILD)

$(MIMALLOC_CONFIG):
	mkdir -p $(MIMALLOC_BUILD) && cd $(MIMALLOC_BUILD) && \
		cmake .. -DCMAKE_INSTALL_PREFIX=$(DST_DIR) \
				 -DCMAKE_C_COMPILER=$(CROSS_COMPILE)-gcc \
				 -DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)-g++

$(MIMALLOC_ELF): libmalloc

$(APP): $(MIMALLOC_ELF)
	$(MAKE) -C $(MIMALLOC_BUILD) install

install::$(APP)
	@ln -sf $(MIMALLOC_BUILD)/mimalloc-test-api $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)/mimalloc-test-api
	@ln -sf $(MIMALLOC_BUILD)/mimalloc-test-api-fill $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)/mimalloc-test-api-fill
	@ln -sf $(MIMALLOC_BUILD)/mimalloc-test-stress $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)/mimalloc-test-stress
	@ln -sf $(MIMALLOC_BUILD)/mimalloc-test-stress-dynamic $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)/mimalloc-test-stress-dynamic