access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/storage/Makefile

access.libname := access
access.deps := storage
access.includes := $(PROJECT_ROOT)
$(eval $(call library,access))
