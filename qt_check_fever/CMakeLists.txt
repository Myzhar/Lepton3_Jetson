cmake_minimum_required(VERSION 3.5)

project(qt_check_fever LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17) # Required to use QThread::create with args
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES Release )
    #message(" * Release Mode")
    add_compile_options(-Wno-deprecated-declarations)
endif()

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo )
    #message(" * Release with Debug Info Mode")
    add_compile_options(-Wno-deprecated-declarations)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug )
    message(" * Debug Mode")
endif()

############################################################
# Dependencies
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ZED 3 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} EXACT REQUIRED)
find_package(OpenCV REQUIRED)

SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
SET(SPECIAL_OS_LIBS "pthread" "X11")
add_definitions(-Wno-write-strings)
set(OGL_LIBS GL GLU)

############################################################
# Includes
include_directories(
    .
    include
    ${CUDA_INCLUDE_DIRS}
    ${ZED_INCLUDE_DIRS}
)

############################################################
# Sources

set(TS_FILES
    lang/qt_check_fever_en_US.ts
)

set(SRC
    src/main.cpp
    src/mainwindow.cpp
    src/zedgrabber.cpp
    src/leptongrabber.cpp
    src/oglrenderer.cpp
)

set(INC
    include/mainwindow.h
    include/zedgrabber.h
    include/leptongrabber.h
    include/oglrenderer.h
)

set(FORMS
    src/mainwindow.ui
)


############################################################
# Executable

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})

add_executable(qt_check_fever
    ${SRC}
    ${INC}
    ${FORMS}
    ${TS_FILES}
)

target_link_libraries(qt_check_fever LINK_PUBLIC
    Qt5::Widgets
    lepton3_grabber
    ${ZED_LIBS}
    ${SPECIAL_OS_LIBS}
    ${OGL_LIBS}
    ${OpenCV_LIBS}
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
