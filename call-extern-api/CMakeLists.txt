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
target_sources(app PRIVATE src/main.c src/dhcp.c)

