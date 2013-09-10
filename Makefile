# GNU Make solution makefile autogenerated by Premake

ifndef config
  config=releaseflto
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),releaseflto)
  dispatch_lib_config = releaseflto
  dispatch_test_config = releaseflto
  storage_lib_config = releaseflto
  access_lib_config = releaseflto
  storage_perf2_config = releaseflto
endif
ifeq ($(config),release)
  dispatch_lib_config = release
  dispatch_test_config = release
  storage_lib_config = release
  access_lib_config = release
  storage_perf2_config = release
endif

PROJECTS := dispatch-lib dispatch-test storage-lib access-lib storage-perf2

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

dispatch-lib: 
ifneq (,$(dispatch_lib_config))
	@echo "==== Building dispatch-lib ($(dispatch_lib_config)) ===="
	@${MAKE} --no-print-directory -C build -f dispatch-lib.make config=$(dispatch_lib_config)
endif

dispatch-test: dispatch-lib
ifneq (,$(dispatch_test_config))
	@echo "==== Building dispatch-test ($(dispatch_test_config)) ===="
	@${MAKE} --no-print-directory -C build -f dispatch-test.make config=$(dispatch_test_config)
endif

storage-lib: 
ifneq (,$(storage_lib_config))
	@echo "==== Building storage-lib ($(storage_lib_config)) ===="
	@${MAKE} --no-print-directory -C build -f storage-lib.make config=$(storage_lib_config)
endif

access-lib: 
ifneq (,$(access_lib_config))
	@echo "==== Building access-lib ($(access_lib_config)) ===="
	@${MAKE} --no-print-directory -C build -f access-lib.make config=$(access_lib_config)
endif

storage-perf2: access-lib storage-lib dispatch-lib
ifneq (,$(storage_perf2_config))
	@echo "==== Building storage-perf2 ($(storage_perf2_config)) ===="
	@${MAKE} --no-print-directory -C build -f storage-perf2.make config=$(storage_perf2_config)
endif

clean:
	@${MAKE} --no-print-directory -C build -f dispatch-lib.make clean
	@${MAKE} --no-print-directory -C build -f dispatch-test.make clean
	@${MAKE} --no-print-directory -C build -f storage-lib.make clean
	@${MAKE} --no-print-directory -C build -f access-lib.make clean
	@${MAKE} --no-print-directory -C build -f storage-perf2.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  releaseflto"
	@echo "  release"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   dispatch-lib"
	@echo "   dispatch-test"
	@echo "   storage-lib"
	@echo "   access-lib"
	@echo "   storage-perf2"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
