NAME = libcunit
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBCUNIT_REPO = $(abspath repo)

.PHONY: libcunit

libcunit:
	cd $(LIBCUNIT_REPO) && \
		libtoolize --force && \
		aclocal && \
		autoheader && \
		automake --force-missing --add-missing && \
		autoconf && \
		./configure --prefix=$(DST_DIR) --host=riscv64-unknown-linux-gnu CFLAGS="-O2 -march=rv64gc -mabi=lp64d"
	cd $(LIBCUNIT_REPO) && make && make install

$(APP): libcunit
# 	ln -sf $(abspath $<) $@