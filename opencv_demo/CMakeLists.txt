SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

find_package(OpenCV REQUIRED)

add_executable (opencv_demo opencv_demo.cpp)
target_link_libraries (opencv_demo LINK_PUBLIC 
                       lepton3_grabber 
                       ${OpenCV_LIBS})
