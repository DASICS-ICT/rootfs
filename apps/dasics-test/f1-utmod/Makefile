obj-m += f1_utmod.o
ccflags-y += -fPIC -g

MODULE_NAME := f1_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
