SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

find_package(OpenCV REQUIRED)

add_executable (check_fever_app check_fever_app.cpp)
target_link_libraries (check_fever_app LINK_PUBLIC 
                       lepton3_grabber 
                       ${OpenCV_LIBS})
