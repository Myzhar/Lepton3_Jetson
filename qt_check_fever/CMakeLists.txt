cmake_minimum_required(VERSION 3.5)

project(qt_check_fever LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17) # Required to use QThread::create with args
set(CMAKE_CXX_STANDARD_REQUIRED ON)

############################################################
# Dependencies
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ZED 3 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} EXACT REQUIRED)

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
    src/grabber.cpp
    src/oglrenderer.cpp
)

set(INC
    include/mainwindow.h
    include/grabber.h
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
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
