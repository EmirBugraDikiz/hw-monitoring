cmake_minimum_required(VERSION 4.2.1)

project(hw_monitoring)

add_library(metrics STATIC src/metrics.c)

target_include_directories(metrics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(hw_monitoring_program src/main.c)

target_link_libraries(hw_monitoring_program PRIVATE metrics)