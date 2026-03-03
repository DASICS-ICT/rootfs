APP_DIR := $(shell pwd)
DST_DIR := $(APP_DIR)/build
IST_DIR ?= $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)

CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
OBJDUMP := $(CROSS_COMPILE)objdump

ifndef LIBDASICS_HOME
$(error LIBDASICS_HOME is not defined)
endif

LIBDASICS_LIB := $(LIBDASICS_HOME)/build/libDASICS.a
LIBDASICS_LDS := $(LIBDASICS_HOME)/ld.lds

TARGET := test_hello_world

CFLAGS += -O2 -g -MMD -DDASICS_LINUX -I$(LIBDASICS_HOME)/include
LDFLAGS += -T$(LIBDASICS_LDS)

.DEFAULT_GOAL := all

.PHONY: all install clean

all: $(DST_DIR)/$(TARGET)

$(DST_DIR):
	@mkdir -p $(DST_DIR)

$(DST_DIR)/$(TARGET).o: src/$(TARGET).c | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/$(TARGET): $(DST_DIR)/$(TARGET).o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

install: all
	@mkdir -p $(IST_DIR)
	@cp -f $(DST_DIR)/$(TARGET) $(IST_DIR)/$(TARGET)
	@chmod +x $(IST_DIR)/$(TARGET)

clean:
	rm -rf $(DST_DIR)
	rm -f $(IST_DIR)/$(TARGET)

DEPS := $(DST_DIR)/$(TARGET).d
-include $(DEPS)
