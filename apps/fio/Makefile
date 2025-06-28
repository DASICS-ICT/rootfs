NAME    = fio
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

FIO_REPO   = $(abspath repo)
FIO_ELF    = $(FIO_REPO)/fio
FIO_CONFIG = $(FIO_REPO)/config-host.mak

CROSS_COMPILE ?= riscv64-unknown-linux-gnu-

.PHONY: fio

fio: $(FIO_CONFIG)
	$(MAKE) -C $(FIO_REPO)

$(FIO_CONFIG):
	cd $(FIO_REPO) && cross_compile=yes ./configure --prefix=$(DST_DIR) --cc=$(CROSS_COMPILE)gcc --cpu=riscv64 --build-static

$(FIO_ELF): fio

$(APP): fio

install:: $(APP)
	ln -sf $(abspath $(FIO_ELF)) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/fio)
