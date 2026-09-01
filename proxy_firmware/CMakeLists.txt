cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set")
endif()

if(NOT DEFINED ENV{PICO_PIO_USB_PATH})
    message(FATAL_ERROR "PICO_PIO_USB_PATH is not set")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(kvm_keyboard_proxy C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory($ENV{PICO_PIO_USB_PATH} pico_pio_usb)

add_executable(kvm_keyboard_proxy
    main.c
    usb_descriptors.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
)

target_compile_definitions(kvm_keyboard_proxy PRIVATE PIO_USB_USE_TINYUSB)
target_include_directories(kvm_keyboard_proxy PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(kvm_keyboard_proxy PRIVATE
    pico_stdlib
    pico_multicore
    pico_pio_usb
    tinyusb_device
    tinyusb_host
)

pico_add_extra_outputs(kvm_keyboard_proxy)
