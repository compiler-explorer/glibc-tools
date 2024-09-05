This is a compiler-explorer fork of the glibc-tools repository, which contains tools and library provided by glibc that
either have been deprectated of moved out from the project. At the time of this fork, it contained only libSegFault.

## Changes to libSegFault
- CMake for building
- Improved stack tracing with [cpptrace](https://github.com/jeremy-rifkin/cpptrace)

## Dependencies

- GCC 11 or higher (or equivalent)
- Cpptrace depends on libdwarf, zlib, zstd, and is configured to use libunwind.

## Contributing

To build locally run `make build`. Example usage:

```
LD_PRELOAD=$LD_PRELOAD:build/libSegFault.so LIBSEGFAULT_TRACER=build/tracer ./some_program
```

## Environment variables

### Required

* LIBSEGFAULT_TRACER
  - Full path to the `tracer` program

### Optional

* SEGFAULT_SIGNALS="..."
  - Space separated list of signals to listen to
  - Supported values are: segv, ill, abrt, fpe, bus, stkflt, all

* LIBSEGFAULT_REGISTERS=1
  - Include the state of all the registers

* LIBSEGFAULT_MEMORY=1
  - Include a memory map

* LIBSEGFAULT_DEBUG=1
  - For situations when libSegFault doesn't work as expected

### Deprecated / Should not use

* SEGFAULT_USE_ALTSTACK
* SEGFAULT_OUTPUT_NAME
