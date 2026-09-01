cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(kvm_switcher C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(kvm_switcher
    main.c
)

target_link_libraries(kvm_switcher
    pico_stdlib
)

pico_enable_stdio_usb(kvm_switcher 1)
pico_enable_stdio_uart(kvm_switcher 0)

pico_add_extra_outputs(kvm_switcher)
