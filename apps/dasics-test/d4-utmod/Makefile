obj-m += d4_utmod.o
ccflags-y += -fPIC -g

MODULE_NAME := d4_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
