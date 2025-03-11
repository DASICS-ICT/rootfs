NAME    = pcredemo
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu
DEMO_SRC = $(APP_DIR)/pcredemo.c
PCRE = $(abspath $(RISCV_ROOTFS_HOME)/libs/pcre/build)
DASICS = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS/build)

$(APP): $(DEMO_SRC)
	$(CROSS_COMPILE)-gcc -Wall -g pcredemo.c -o $(APP) \
		-I$(PCRE)/include -L$(PCRE)/lib -lpcre \
		-I$(DASICS)/include -L$(DASICS) -lDASICS -T$(DASICS)/ld.lds

