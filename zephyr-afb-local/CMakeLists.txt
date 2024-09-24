# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

#list(APPEND EXTRA_ZEPHYR_MODULES "$ENV{HOME}/zephyrproject/afb-zephyr")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(afb-local)

target_sources(app PRIVATE src/main.c)
