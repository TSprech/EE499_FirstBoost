cmake_minimum_required(VERSION 3.24)
include(ARMGCCToolchain.cmake)

include(pico-sdk/pico_sdk_init.cmake)

project(FirstBoost C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif ()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
    add_executable(FirstBoost Src/main.cpp)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(FirstBoost 1)
    pico_enable_stdio_uart(FirstBoost 0)

    # pull in common dependencies
    target_link_libraries(FirstBoost pico_stdlib pico_multicore)

    # pull in common dependencies and additional pwm hardware support
    target_link_libraries(FirstBoost pico_stdlib hardware_pwm hardware_adc)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(FirstBoost)

    target_include_directories(FirstBoost PUBLIC Inc)
    add_compile_definitions(JSON_NO_IO=1)
    add_link_options(--print-memory-usage)
elseif (PICO_ON_DEVICE)
    message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif ()