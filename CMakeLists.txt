cmake_minimum_required(VERSION 3.14)

project(
  glibc-tools
  VERSION 0.0.1
  LANGUAGES C CXX
)

set(ARCH "x86_64" CACHE STRING "Architecture to build for.")

add_library(
  SegFault
  SHARED
  libSegFault/segfault.c
  libSegFault/_itoa.c
)

target_include_directories(SegFault PRIVATE libSegFault/${ARCH} libSegFault/) # Note: Order is important
target_compile_definitions(SegFault PRIVATE _GNU_SOURCE)
