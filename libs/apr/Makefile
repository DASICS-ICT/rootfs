NAME    = libapr-1.a
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu-

APR_REPO   = $(abspath repo)
APR_CONFIG = $(APR_REPO)/config.status
APR_A      = $(APR_REPO)/.libs/libapr-1.a

.PHONY: apr

apr: $(APR_CONFIG)
	$(MAKE) -C $(APR_REPO)

$(APR_CONFIG):
	cd $(APR_REPO) && ./buildconf && \
		./configure --prefix=$(DST_DIR) --host=riscv64 CC=$(CROSS_COMPILE)gcc \
			ac_cv_file__dev_zero=yes \
			ac_cv_func_setpgrp_void=yes \
			apr_cv_tcp_nodelay_with_cork=yes \
			apr_cv_process_shared_works=yes \
			apr_cv_mutex_robust_shared=yes

$(APR_A): apr

$(APP): $(APR_A)
	$(MAKE) -C $(APR_REPO) install