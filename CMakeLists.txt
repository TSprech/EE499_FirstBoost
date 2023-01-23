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

add_executable(FirstBoost
        Src/Blink.cpp
        )

# pull in common dependencies
target_link_libraries(FirstBoost pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(FirstBoost)

## add url via pico_set_program_url
#example_auto_set_url(FirstBoost)
