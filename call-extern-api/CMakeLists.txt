#
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# basic detection of afb-zephyr
if("" STREQUAL "$ENV{AFB_ZEPHYR_DIR}")
   set(ENV{AFB_ZEPHYR_DIR} "$ENV{HOME}/zephyrproject/afb-zephyr")
endif()
list(APPEND EXTRA_ZEPHYR_MODULES "$ENV{AFB_ZEPHYR_DIR}")

# finding Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# set project
project(call-extern-api)
target_sources(app PRIVATE src/main.c ../common/dhcp.c)
if (CONFIG_MBEDTLS)
   generate_inc_file_for_target(app src/cert.der ${ZEPHYR_BINARY_DIR}/include/generated/trust.h)
   generate_inc_file_for_target(app src/cert.der ${ZEPHYR_BINARY_DIR}/include/generated/cert.h)
   generate_inc_file_for_target(app src/key.der ${ZEPHYR_BINARY_DIR}/include/generated/key.h)
endif()
