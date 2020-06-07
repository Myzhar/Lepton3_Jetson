SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

#########################################################
# SOURCES
set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lepton3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/src/stopwatch.cpp
)

# HEADERS
set( INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lepton3.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/include/stopwatch.hpp
)
#########################################################

# FLIR Lepton SDK
add_subdirectory(Lepton_SDK)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Lepton_SDK/include
)

add_library( lepton3_grabber ${SRC} )
target_link_libraries( lepton3_grabber LINK_PUBLIC 
                       lepton_sdk
                       -ldl
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories( lepton3_grabber PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/tools/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/Lepton_SDK/include
)
