cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(pio_switch C CXX ASM)

pico_sdk_init()

add_executable(pio_switch)

pico_generate_pio_header(pio_switch ${CMAKE_CURRENT_LIST_DIR}/switches.pio)

target_sources(pio_switch PRIVATE pio_switch.c)

target_link_libraries(pio_switch PRIVATE
        pico_stdlib
        hardware_pio
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(pio_switch 1)
pico_enable_stdio_uart(pio_switch 0)

pico_add_extra_outputs(pio_switch)