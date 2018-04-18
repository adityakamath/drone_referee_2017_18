cmake_minimum_required(VERSION 2.8)
project(project_012_Aruco_UDP)

find_package( OpenCV REQUIRED )

add_executable(main src/main.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries( main ${OpenCV_LIBS} )

set(CMAKE_BUILD_TYPE Debug)
