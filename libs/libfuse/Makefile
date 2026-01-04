NAME = libfuse3.a
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

BUILD_NAME = riscv64-build-gcc
LIBFUSE_REPO = $(abspath repo)

.PHONY: libfuse

libfuse:
	cd $(LIBFUSE_REPO) && \
	meson setup --prefix=$(DST_DIR) $(BUILD_NAME) --cross-file ../riscv64_linux_gcc.cfg \
		-Ddefault_library=both -Duseroot=false -Dinitscriptdir=
	cd $(LIBFUSE_REPO) && \
	ninja -C $(BUILD_NAME) && \
	ninja -C $(BUILD_NAME) install

$(APP): libfuse
# 	ln -sf $(abspath $<) $@