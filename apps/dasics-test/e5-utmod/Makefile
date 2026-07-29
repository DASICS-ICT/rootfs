obj-m += e5_utmod.o
e5_utmod-y := module.o entries.o
ccflags-y += -fPIC -g

MODULE_NAME := e5_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
