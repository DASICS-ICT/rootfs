obj-m += dummy_logic.o
dummy_logic-objs := module.o maincall.o
ccflags-y += -fPIC -g -I$(src)/..

MODULE_NAME := dummy_logic
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
