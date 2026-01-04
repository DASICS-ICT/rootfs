NAME = libnuma
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBNUMA_REPO = $(abspath repo)

.PHONY: libnuma

libnuma:
	cd $(LIBNUMA_REPO) && ./autogen.sh && autoconf -i
	cd $(LIBNUMA_REPO) && ./configure --host=riscv64-unknown-linux-gnu CC=riscv64-unknown-linux-gnu-gcc --prefix=$(DST_DIR) CFLAGS="-O2 -march=rv64gc -mabi=lp64d" && make -j install

$(APP): libnuma
# 	ln -sf $(abspath $<) $@