perf := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/access/Makefile $(PROJECT_ROOT)/storage/Makefile

perf.binname := perf
perf.deps := access storage
$(eval $(call full_link_binary,perf))
