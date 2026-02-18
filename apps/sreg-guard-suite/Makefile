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

COMMON_OBJS := \
	$(DST_DIR)/sreg_guard_suite_common.o \
	$(DST_DIR)/sreg_untrusted.o

TARGETS := \
	test_sreg_guard_suite

CFLAGS += -O2 -g -MMD -fomit-frame-pointer -DDASICS_LINUX -I$(APP_DIR)/include -I$(LIBDASICS_HOME)/include
LDFLAGS += -T$(LIBDASICS_LDS)

.DEFAULT_GOAL := all

.PHONY: all install clean

all: $(addprefix $(DST_DIR)/,$(TARGETS))

$(DST_DIR):
	@mkdir -p $(DST_DIR)

$(DST_DIR)/%.o: src/%.c | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/%.o: src/%.S | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/sreg_guard_suite_main.o: src/sreg_guard_suite_main.c | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/test_sreg_guard_suite: $(COMMON_OBJS) $(DST_DIR)/sreg_guard_suite_main.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

install: all
	@mkdir -p $(IST_DIR)
	@rm -f $(IST_DIR)/test_s0_guard_suite
	@for bin in $(TARGETS); do \
		cp -f $(DST_DIR)/$$bin $(IST_DIR)/$$bin; \
		chmod +x $(IST_DIR)/$$bin; \
	done

clean:
	rm -rf $(DST_DIR)
	rm -f $(IST_DIR)/test_sreg_guard_suite $(IST_DIR)/test_s0_guard_suite

DEPS := \
	$(DST_DIR)/sreg_guard_suite_common.d \
	$(DST_DIR)/sreg_guard_suite_main.d \
	$(DST_DIR)/sreg_untrusted.d
-include $(DEPS)
