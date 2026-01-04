NAME = libfuse3.a

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

BUILD_NAME = riscv64-build-gcc

LIBFUSE_REPO = $(abspath repo)
LIBFUSE_A    = $(LIBFUSE_REPO)/$(BUILD_NAME)/lib/libfuse3.a

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)

.PHONY: libfuse

libfuse:
	cd $(LIBFUSE_REPO) && \
	meson setup --prefix=$(LIBFUSE_REPO)/build riscv64-build-gcc --cross-file ../riscv64_linux_gcc.cfg \
		-Ddefault_library=both -Duseroot=false -Dinitscriptdir=
	cd $(LIBFUSE_REPO) && \
	ninja -C $(BUILD_NAME) && \
	ninja -C $(BUILD_NAME) install

$(LIBFUSE_A): libfuse

$(APP): $(LIBFUSE_A)
	ln -sf $(abspath $<) $@