# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#
# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).
set(BOARD_ROOT .)

add_subdirectory(common)

zephyr_include_directories(
    include,
    rp2040-app,
    esp32-app,
    common)
