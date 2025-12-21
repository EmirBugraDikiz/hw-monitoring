cmake_minimum_required(VERSION 3.16)

project(hw_monitoring C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# libgpiod (pkg-config ile bul)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_library(hardware_monitoring_lib STATIC
    src/hardware_stats.c
    src/page_manager.c
    src/utility.c
    src/input/buttons.c
    src/lcd/hd44780.c
)

target_include_directories(hardware_monitoring_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${GPIOD_INCLUDE_DIRS}
)

target_compile_options(hardware_monitoring_lib PRIVATE
    -Wall -Wextra -Wpedantic
    ${GPIOD_CFLAGS_OTHER}
)

target_link_libraries(hardware_monitoring_lib PUBLIC
    ${GPIOD_LIBRARIES}
)

add_executable(hw_monitoring_program
    src/main.c
)

target_link_libraries(hw_monitoring_program PRIVATE
    hardware_monitoring_lib
)
