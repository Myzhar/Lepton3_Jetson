cmake_minimum_required (VERSION 2.8.9)

project (lepton3_jetson)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_DEBUG_POSTFIX "_d")

#########################################################
# FLAGS
set (CMAKE_CXX_STANDARD 11)
#########################################################

# Letpon3 Library source code
add_subdirectory (grabber_lib)

# OpenCV demo
add_subdirectory (opencv_demo)


