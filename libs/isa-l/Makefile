NAME = libisal
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBISAL_REPO = $(abspath repo)

.PHONY: libisal

libisal:
	cd $(LIBISAL_REPO) && ./autogen.sh
	cd $(LIBISAL_REPO) && ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes ./configure --prefix=$(DST_DIR) --libdir=$(DST_DIR)/lib --host=riscv64-unknown-linux-gnu CFLAGS="-O2 -march=rv64gc -mabi=lp64d"
	cd $(LIBISAL_REPO) && make && make install

$(APP): libisal
# 	ln -sf $(abspath $<) $@