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
	$(DST_DIR)/m3_padding.o \
	$(DST_DIR)/m3_negative_untrusted.o \
	$(DST_DIR)/m3_case01_entry.o

TARGETS := \
	test_m3_cprog_feas \
	test_m3_cprog_feas_rvc

CFLAGS += -O2 -g -MMD -fomit-frame-pointer -DDASICS_LINUX -I$(APP_DIR)/include -I$(LIBDASICS_HOME)/include
LDFLAGS += -T$(LIBDASICS_LDS)
NORVC_CFLAGS := $(CFLAGS) -march=rv64imafdzicsr_zifencei -mabi=lp64d
RVC_CFLAGS := $(CFLAGS) -march=rv64imafdc_zicsr_zifencei -mabi=lp64d

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
	@$(CC) $(NORVC_CFLAGS) -c -o $@ $<

$(DST_DIR)/m3_case01_entry.o: src/m3_case01_entry.c | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(NORVC_CFLAGS) -c -o $@ $<

$(DST_DIR)/%_rvc.o: src/%.c | $(DST_DIR)
	@echo + CC $<
	@$(CC) $(RVC_CFLAGS) -c -o $@ $<

$(DST_DIR)/test_m3_cprog_feas: $(COMMON_OBJS) $(DST_DIR)/test_m3_cprog_feas.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

$(DST_DIR)/test_m3_cprog_feas_rvc: \
	$(DST_DIR)/m3_padding.o \
	$(DST_DIR)/m3_negative_untrusted.o \
	$(DST_DIR)/m3_case01_entry_rvc.o \
	$(DST_DIR)/test_m3_cprog_feas_rvc.o
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBDASICS_LIB)
	@$(OBJDUMP) -d $@ > $@.txt

install: all
	@mkdir -p $(IST_DIR)
	@for bin in $(TARGETS); do \
		cp -f $(DST_DIR)/$$bin $(IST_DIR)/$$bin; \
		chmod +x $(IST_DIR)/$$bin; \
	done

clean:
	rm -rf $(DST_DIR)
	rm -f $(IST_DIR)/test_m3_cprog_feas $(IST_DIR)/test_m3_cprog_feas_rvc

DEPS := \
	$(DST_DIR)/m3_padding.d \
	$(DST_DIR)/m3_negative_untrusted.d \
	$(DST_DIR)/m3_case01_entry.d \
	$(DST_DIR)/m3_case01_entry_rvc.d \
	$(DST_DIR)/test_m3_cprog_feas.d \
	$(DST_DIR)/test_m3_cprog_feas_rvc.d
-include $(DEPS)
