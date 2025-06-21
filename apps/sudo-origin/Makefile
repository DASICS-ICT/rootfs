NAME    = sudo-origin
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif


SUDO_REPO   = $(abspath repo)
SUDO_ELF    = $(SUDO_REPO)/sudo/src/sudo-origin

.PHONY:  sudo-origin

sudo-origin:
	$(MAKE) -C $(SUDO_REPO)

$(SUDO_ELF):  sudo-origin


$(APP): $(SUDO_ELF)
	ln -sf $(abspath $(SUDO_REPO)/build/exploit) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/home/ceshi/exploit)


install:: $(APP)
	ln -sf $(abspath $(SUDO_REPO)/sudo/src/sudo) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/sudo-origin)