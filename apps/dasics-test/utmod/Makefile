obj-m += utmod.o
ccflags-y += -fPIC -g

MODULE_NAME := utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
