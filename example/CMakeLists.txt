cmake_minimum_required(VERSION 3.10)
project(OpfBenchSDK)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# MinGW compatibility
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wno-narrowing -Wno-attributes)
endif()

# SDK layout: the parent of example/ is the SDK root
set(SDK_ROOT ${CMAKE_SOURCE_DIR}/..)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(WIN32)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/POICache)
endif()


# Header search paths:
#   libOptInterface headers: ${SDK_ROOT}/include
#   libOptInterface third-party (cppad/ipopt/tcc/ankerl): ${SDK_ROOT}/include/thirdparty/*
#   example private headers: ${CMAKE_SOURCE_DIR}/include
include_directories(
    ${SDK_ROOT}/include
    ${SDK_ROOT}/include/thirdparty
    ${SDK_ROOT}/include/thirdparty/cppad
    ${SDK_ROOT}/include/thirdparty/ipopt
    ${SDK_ROOT}/include/thirdparty/tcc
    ${CMAKE_SOURCE_DIR}/include
)

# acopf_test executable
add_executable(acopf_test
    src/acopf_test.cpp
)

# Link libOptInterface (pass DLL directly; MinGW auto-finds .dll.a next to the DLL)
if(WIN32)
  target_link_libraries(acopf_test PRIVATE ${SDK_ROOT}/lib/libOptInterface.dll)
else()
  target_link_libraries(acopf_test PRIVATE ${SDK_ROOT}/lib/libOptInterface.so tcc)
endif()

# Copy runtime DLL next to build/bin/
if(WIN32)
  add_custom_command(TARGET acopf_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${SDK_ROOT}/lib/libOptInterface.dll
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libOptInterface.dll
    COMMENT "Copying libOptInterface.dll to output dir")
endif()

message(STATUS "OPF SDK example build configured")
message(STATUS "  SDK root: ${SDK_ROOT}")
message(STATUS "  Include : ${SDK_ROOT}/include")
message(STATUS "  Lib     : ${SDK_ROOT}/lib/libOptInterface.dll")