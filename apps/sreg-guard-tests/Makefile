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
	$(DST_DIR)/sreg_guard_common.o \
	$(DST_DIR)/sreg_regs.o \
	$(DST_DIR)/sreg_untrusted.o

TARGETS := \
	test_sreg_guard_no_protect \
	test_sreg_guard_eager \
	test_sreg_guard_lazy \
	test_sreg_guard_proposed

CFLAGS += -O2 -g -MMD -DDASICS_LINUX -I$(APP_DIR)/include -I$(LIBDASICS_HOME)/include
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

$(DST_DIR)/test_sreg_guard_no_protect: $(COMMON_OBJS) $(DST_DIR)/test_sreg_guard_no_protect.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

$(DST_DIR)/test_sreg_guard_eager: $(COMMON_OBJS) $(DST_DIR)/test_sreg_guard_eager.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

$(DST_DIR)/test_sreg_guard_lazy: $(COMMON_OBJS) $(DST_DIR)/test_sreg_guard_lazy.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

$(DST_DIR)/test_sreg_guard_proposed: $(COMMON_OBJS) $(DST_DIR)/test_sreg_guard_proposed.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

install: all
	@mkdir -p $(IST_DIR)
	@for bin in $(TARGETS); do \
		cp -f $(DST_DIR)/$$bin $(IST_DIR)/$$bin; \
		chmod +x $(IST_DIR)/$$bin; \
	done
	@ln -sf test_sreg_guard_proposed $(IST_DIR)/test_dasics

clean:
	rm -rf $(DST_DIR)
	rm -f $(IST_DIR)/test_sreg_guard_no_protect \
		$(IST_DIR)/test_sreg_guard_eager \
		$(IST_DIR)/test_sreg_guard_lazy \
		$(IST_DIR)/test_sreg_guard_proposed \
		$(IST_DIR)/test_dasics

DEPS := $(wildcard $(DST_DIR)/*.d)
-include $(DEPS)
