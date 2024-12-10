NAME    = libatomic_ops.a
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBATOMIC_OPS_REPO   = $(abspath repo)
LIBATOMIC_OPS_CONFIG = $(LIBATOMIC_OPS_REPO)/Makefile
LIBATOMIC_OPS_A      = $(LIBATOMIC_OPS_REPO)/.libs/libatomic_ops.a

.PHONY: libatomic_ops

libatomic_ops: $(LIBATOMIC_OPS_CONFIG)
	$(MAKE) -C $(LIBATOMIC_OPS_REPO)

$(LIBATOMIC_OPS_CONFIG):
	cd $(LIBATOMIC_OPS_REPO) && ./autogen.sh && ./configure --prefix=$(DST_DIR) CROSS_CC=riscv64-unknown-linux-gnu-gcc

$(LIBATOMIC_OPS_A): libatomic_ops

$(APP): $(LIBATOMIC_OPS_A)
	$(MAKE) -C $(LIBATOMIC_OPS_REPO) install