cmake_minimum_required (VERSION 3.14)

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

# Check Fever App
add_subdirectory (check_fever_app)

# Qt ZED app
find_package(Qt5 COMPONENTS Widgets)
find_package(ZED 3)
if(Qt5_FOUND AND ZED_FOUND)
   add_subdirectory(qt_check_fever)
endif()
