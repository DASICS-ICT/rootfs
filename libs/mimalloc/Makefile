NAME    = libmimalloc.so.2
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/libmimalloc.so.2.2

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CLANG_TARGET      = --target=riscv64-unknown-linux-gnu
CLANG_SYSROOT     = --sysroot=$(RISCV)/sysroot
CLANG_GCC_TOOLCHAIN = --gcc-toolchain=$(RISCV)

MIMALLOC_REPO   = $(abspath repo)
MIMALLOC_BUILD  = $(MIMALLOC_REPO)/build
MIMALLOC_CONFIG = $(MIMALLOC_BUILD)/Makefile
MIMALLOC_ELF    = $(MIMALLOC_BUILD)/libmimalloc.so.2.2

.PHONY: libmalloc

libmalloc: $(MIMALLOC_CONFIG)
	$(MAKE) -C $(MIMALLOC_BUILD)

$(MIMALLOC_CONFIG):
	mkdir -p $(MIMALLOC_BUILD) && cd $(MIMALLOC_BUILD) && \
		cmake .. -DCMAKE_INSTALL_PREFIX=$(DST_DIR) \
				 -DCMAKE_C_COMPILER=clang \
				 -DCMAKE_CXX_COMPILER=clang++ \
				 -DCMAKE_AR=$(shell which llvm-ar) \
				 -DCMAKE_RANLIB=$(shell which llvm-ranlib) \
				 -DCMAKE_C_FLAGS="$(CLANG_TARGET) $(CLANG_SYSROOT) $(CLANG_GCC_TOOLCHAIN)" \
				 -DCMAKE_CXX_FLAGS="$(CLANG_TARGET) $(CLANG_SYSROOT) $(CLANG_GCC_TOOLCHAIN)" \
				 -DCMAKE_EXE_LINKER_FLAGS="$(CLANG_TARGET) $(CLANG_SYSROOT) $(CLANG_GCC_TOOLCHAIN)" \
				 -DCMAKE_SHARED_LINKER_FLAGS="$(CLANG_TARGET) $(CLANG_SYSROOT) $(CLANG_GCC_TOOLCHAIN)" \
				 -DCMAKE_SYSTEM_NAME=Linux \
				 -DCMAKE_SYSTEM_PROCESSOR=riscv64

$(MIMALLOC_ELF): libmalloc

$(APP): $(MIMALLOC_ELF)
	$(MAKE) -C $(MIMALLOC_BUILD) install
