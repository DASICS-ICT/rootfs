NAME = sqlite3-test
SRCS = sqlite3-test.c

SQLITE_BUILD = $(RISCV_ROOTFS_HOME)/libs/sqlite3/build

INC_DIR += $(SQLITE_BUILD)/include
LDFLAGS += -L$(SQLITE_BUILD)/lib -lsqlite3 -Wl,-rpath,$(SQLITE_BUILD)/lib

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif
