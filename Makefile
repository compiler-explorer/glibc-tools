default: help

help: # with thanks to Ben Rady
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

.PHONY: build
build: debug  ## build in debug mode

build/configured-debug:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On
	rm -f build/configured-release
	touch build/configured-debug

build/configured-release:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On
	rm -f build/configured-debug
	touch build/configured-release

.PHONY: configure-debug
configure-debug: build/configured-debug

.PHONY: configure-release
configure-release: build/configured-release

.PHONY: debug
debug: configure-debug  ## build in debug mode
	cmake --build build

.PHONY: release
release: configure-release  ## build in release mode (with debug info)
	cmake --build build

.PHONY: clean
clean:  ## clean
	rm -rf build