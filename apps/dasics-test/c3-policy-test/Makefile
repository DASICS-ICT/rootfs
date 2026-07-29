obj-m += c3_policy_test.o
ccflags-y += -fPIC -g

MODULE_NAME := c3_policy_test
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
