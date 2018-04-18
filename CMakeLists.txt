cmake_minimum_required(VERSION 2.8)
project(droneref_cv)

find_package( OpenCV REQUIRED )

add_executable(ball_detect_red src/ball_detect_red.cpp)
add_executable(ball_detect_yellow src/ball_detect_yellow.cpp)
add_executable(aruco_detect src/aruco_detect.cpp)
add_executable(player_tracker src/player_tracker.cpp)
add_executable(aruco_test src/aruco_test.cpp)
add_executable(peters_code src/peters_code.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries( ball_detect_red ${OpenCV_LIBS} )
target_link_libraries( ball_detect_yellow ${OpenCV_LIBS} )
target_link_libraries( ball_detect ${OpenCV_LIBS} )
target_link_libraries( player_tracker ${OpenCV_LIBS} )
target_link_libraries(aruco_test ${OpenCV_LIBS})
target_link_libraries(peters_code ${OpenCV_LIBS})

set(CMAKE_BUILD_TYPE Debug)
