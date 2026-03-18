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
	$(DST_DIR)/m4_sreg_alpha_suite_common.o \
	$(DST_DIR)/m4_untrusted.o \
	$(DST_DIR)/m4_migrated_cases.o \
	$(DST_DIR)/m4_trusted.o

TARGETS := \
	test_m4_sreg_alpha_suite

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
	@echo + AS $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/test_m4_sreg_alpha_suite: $(COMMON_OBJS) $(DST_DIR)/m4_sreg_alpha_suite_main.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

install: all
	@mkdir -p $(IST_DIR)
	@cp -f $(DST_DIR)/test_m4_sreg_alpha_suite $(IST_DIR)/test_m4_sreg_alpha_suite
	@chmod +x $(IST_DIR)/test_m4_sreg_alpha_suite

clean:
	rm -rf $(DST_DIR)
	rm -f $(IST_DIR)/test_m4_sreg_alpha_suite

DEPS := \
	$(DST_DIR)/m4_sreg_alpha_suite_common.d \
	$(DST_DIR)/m4_migrated_cases.d \
	$(DST_DIR)/m4_sreg_alpha_suite_main.d \
	$(DST_DIR)/m4_trusted.d \
	$(DST_DIR)/m4_untrusted.d
-include $(DEPS)
