access := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/storage/Makefile
include $(PROJECT_ROOT)/dispatch/Makefile

access.libname := access
access.deps := storage dispatch
access.includes := $(PROJECT_ROOT)
$(eval $(call library,access))
