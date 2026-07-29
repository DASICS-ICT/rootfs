obj-m += a2_utmod.o
a2_utmod-y := module.o entries.o
ccflags-y += -fPIC -g

MODULE_NAME := a2_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
