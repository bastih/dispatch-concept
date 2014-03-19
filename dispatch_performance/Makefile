dispatch_performance := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/dispatch2/Makefile

dispatch_performance.binname := dispatch_performance
dispatch_performance.deps := dispatch2
$(eval $(call full_link_binary,dispatch_performance))
