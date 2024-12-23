NAME    = httperf
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)
APP     = $(DST_DIR)/bin/httperf

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu



HTTPERF_REPO   = $(abspath repo)
HTTPERF_CONFIG = $(HTTPERF_REPO)/Makefile
HTTPERF_ELF    = $(HTTPERF_REPO)/build/bin/httperf

.PHONY: httperf

httperf: $(HTTPERF_CONFIG)
	$(MAKE) -C $(HTTPERF_REPO)

$(HTTPERF_CONFIG):
	cd $(HTTPERF_REPO) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) CFLAGS="-I$(RISCV_ROOTFS_HOME)/libs/openssl/build/include" LDFLAGS="-L$(RISCV_ROOTFS_HOME)/libs/openssl/build/lib"

$(HTTPERF_ELF): httperf

$(APP): $(HTTPERF_ELF)
	$(MAKE) -C $(HTTPERF_REPO) install

install:: $(APP)
	ln -sf $(DST_DIR)/bin/httperf $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin/httperf)