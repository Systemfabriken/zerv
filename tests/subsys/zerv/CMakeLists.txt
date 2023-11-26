# Copyright (c) 2023 Systemfabriken AB
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_unit)

target_sources(app PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/test_main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/zerv_test_service.c
  ${CMAKE_CURRENT_SOURCE_DIR}/zerv_test_service_poll.c
)

target_include_directories(app PRIVATE 
  .
  ..
)
