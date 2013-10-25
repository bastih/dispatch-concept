dispatch := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

dispatch.libname := dispatch
dispatch.includes := $(PROJECT_ROOT)
$(eval $(call library,dispatch))
