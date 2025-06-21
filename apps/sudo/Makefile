NAME    = sudo
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

DEFAULT_CONFIG = config

SUDO_REPO   = $(abspath repo)
SUDO_ELF    = $(SUDO_REPO)/build/sudo

.PHONY: sudo

sudo:
	$(MAKE) -C $(SUDO_REPO)



$(SUDO_ELF): sudo

$(APP): $(SUDO_ELF)
	ln -sf $(abspath $(SUDO_REPO)/build/exploit) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/home/ceshi/exploit-defend)
	ln -sf $(abspath $(SUDO_REPO)/build/libnss_x-x.so.2) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib/libnss_x-x.so.2)

install:: $(APP)
	ln -sf $(abspath $(SUDO_ELF)) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/sudo)

