#########################################################
# SOURCES
file(GLOB_RECURSE SDK_SRC *.c *.cpp)
# HEADERS
file(GLOB_RECURSE SDK_HDR *.h *.hpp)
#########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library( lepton_sdk ${SDK_SRC} )

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories( lepton_sdk PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
)
