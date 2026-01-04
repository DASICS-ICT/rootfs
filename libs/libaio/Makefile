NAME = libaio
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBAIO_REPO = $(abspath repo)

.PHONY: libaio

libaio:
	cd $(LIBAIO_REPO) && \
		CC=riscv64-unknown-linux-gnu-gcc CXX=riscv64-unknown-linux-gnu-g++ LD=riscv64-unknown-linux-gnu-ld make -j &&\
		make -j install DESTDIR=$(DST_DIR)

$(APP): libaio
# 	ln -sf $(abspath $<) $@