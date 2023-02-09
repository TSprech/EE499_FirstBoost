cmake_minimum_required(VERSION 3.24)

add_library(SerialTransferCpp Millis.hpp PacketCpp.cpp PacketCpp.hpp PacketCRCCpp.hpp SerialConfig.hpp SerialTransferCpp.cpp SerialTransferCpp.hpp)

target_include_directories(SerialTransferCpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
