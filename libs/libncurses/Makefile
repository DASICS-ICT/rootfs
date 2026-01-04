NAME = libncurses
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBNCURSES_REPO = $(abspath repo)

.PHONY: libncurses

libncurses:
	cd $(LIBNCURSES_REPO) && \
	./configure --prefix=$(DST_DIR) --host=riscv64-unknown-linux-gnu CFLAGS="-O2 -march=rv64gc -mabi=lp64d" --disable-stripping
	cd $(LIBNCURSES_REPO) && make && make install

$(APP): libncurses
# 	ln -sf $(abspath $<) $@