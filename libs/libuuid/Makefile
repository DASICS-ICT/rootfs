NAME = libuuid
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBUUID_REPO = $(abspath repo)

.PHONY: libuuid

libuuid:
	cd $(LIBUUID_REPO) && \
	./configure --prefix=$(DST_DIR) --host=riscv64-unknown-linux-gnu CFLAGS="-O2 -march=rv64gc -mabi=lp64d"
	cd $(LIBUUID_REPO) && make && make install

$(APP): libuuid
# 	ln -sf $(abspath $<) $@