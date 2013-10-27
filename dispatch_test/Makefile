test := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/dispatch/Makefile

test.binname := test
test.deps := dispatch
test.includes := $(PROJECT_ROOT)/thirdparty/catch/single_include
$(eval $(call binary,test))
