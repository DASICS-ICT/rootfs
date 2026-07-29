obj-m += e1_utmod.o
ccflags-y += -fPIC -g

MODULE_NAME := e1_utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
