cmake_minimum_required(VERSION 3.14)

project(
  glibc-tools
  VERSION 0.0.1
  LANGUAGES C CXX
)

set(ARCH "x86_64" CACHE STRING "Architecture to build for.")
set(CPPTRACE_TAG "v0.8.2" CACHE STRING "Cpptrace git tag")

include(FetchContent)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        ${CPPTRACE_TAG}
)
FetchContent_MakeAvailable(cpptrace)

add_library(
  SegFault
  SHARED
  libSegFault/config.cpp
  libSegFault/segfault.cpp
  libSegFault/_itoa.cpp
  libSegFault/signal-safe-trace.cpp
)
target_include_directories(SegFault PRIVATE libSegFault/${ARCH} libSegFault/) # Note: Order is important
target_compile_definitions(SegFault PRIVATE _GNU_SOURCE)
target_link_libraries(SegFault cpptrace::cpptrace)
target_compile_features(SegFault PRIVATE cxx_std_20)

add_executable(
  tracer
  tracer/main.cpp
)
target_link_libraries(tracer cpptrace::cpptrace)
target_compile_features(tracer PRIVATE cxx_std_20)

add_executable(
  faulty-example
  faulty-example/faulty-example.cpp
)
target_compile_features(faulty-example PRIVATE cxx_std_11)
