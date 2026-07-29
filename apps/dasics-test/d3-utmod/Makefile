obj-m += d3_utmod.o
ccflags-y += -fPIC -g

MODULE_NAME := d3_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
