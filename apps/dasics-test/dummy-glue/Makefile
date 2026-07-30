obj-m += dummy_glue.o
ccflags-y += -fPIC -g -I$(src)/..

MODULE_NAME := dummy_glue
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
