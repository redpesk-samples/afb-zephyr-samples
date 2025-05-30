# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

project(ext2)

# Include EDK CFLAGS
if(NOT DEFINED LLEXT_EDK_INSTALL_DIR)
    set(LLEXT_EDK_INSTALL_DIR $ENV{LLEXT_EDK_INSTALL_DIR})
endif()
include(${LLEXT_EDK_INSTALL_DIR}/cmake.cflags)

# Add LLEXT_CFLAGS to our flags
add_compile_options(${LLEXT_CFLAGS})
add_compile_options("-c")

# Get flags from COMPILE_OPTIONS
get_property(COMPILE_OPTIONS_PROP DIRECTORY PROPERTY COMPILE_OPTIONS)

add_custom_command(
    OUTPUT
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.llext
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.inc
    COMMAND ${CMAKE_C_COMPILER} ${COMPILE_OPTIONS_PROP}
        -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.llext
        ${PROJECT_SOURCE_DIR}/src/main.c
    COMMAND xxd -ip ${PROJECT_NAME}.llext
        ${PROJECT_NAME}.inc
)

add_custom_target(ext2 ALL DEPENDS ${PROJECT_BINARY_DIR}/ext2.llext)
