cmake_minimum_required(VERSION 3.20.0)

FILE(GLOB app_sources
    msg.c
)

target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
