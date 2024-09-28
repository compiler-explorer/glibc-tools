default: help

help: # with thanks to Ben Rady
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

.PHONY: build
build: debug  ## build in debug mode

build/configured-debug:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On
	rm -f build/configured-release
	touch build/configured-debug

build/configured-debug-arm64:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On -DARCH=aarch64
	rm -f build/configured-release-arm64
	touch build/configured-debug-arm64

build/configured-debug-arm64-mac:
	CXXFLAGS="-D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE" cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On -DARCH=aarch64
	rm -f build/configured-release-arm64-mac
	touch build/configured-debug-arm64-debug

build/configured-release:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On
	rm -f build/configured-debug
	touch build/configured-release

build/configured-release-arm64:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On -DARCH=aarch64
	rm -f build/configured-debug-arm64
	touch build/configured-release-arm64

build/configured-release-arm64-mac:
	CXXFLAGS="-D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE" cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCPPTRACE_UNWIND_WITH_LIBUNWIND=On -DARCH=aarch64
	rm -f build/configured-debug-arm64-mac
	touch build/configured-release-arm64-mac

.PHONY: configure-debug
configure-debug: build/configured-debug

.PHONY: configure-debug-arm64
configure-debug-arm64: build/configured-debug-arm64

.PHONY: configure-debug-arm64-mac
configure-debug-arm64-mac: build/configured-debug-arm64-mac

.PHONY: configure-release
configure-release: build/configured-release

.PHONY: configure-release-arm64
configure-release-arm64: build/configured-release-arm64

.PHONY: configure-release-arm64-mac
configure-release-arm64-mac: build/configured-release-arm64-mac

.PHONY: debug
debug: configure-debug  ## build in debug mode
	cmake --build build

.PHONY: debug-arm64
debug: configure-debug-arm64  ## build in debug mode
	cmake --build build

.PHONY: debug-arm64-mac
debug-arm64-mac: configure-debug-arm64-mac  ## build in release mode (with debug info)
	cmake --build build

.PHONY: release
release: configure-release  ## build in release mode (with debug info)
	cmake --build build

.PHONY: release-arm64
release-arm64: configure-release-arm64  ## build in release mode (with debug info)
	cmake --build build

.PHONY: release-arm64-mac
release-arm64-mac: configure-release-arm64-mac  ## build in release mode (with debug info)
	cmake --build build

.PHONY: clean
clean:  ## clean
	rm -rf build
