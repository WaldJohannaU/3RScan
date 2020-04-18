cmake_minimum_required(VERSION 3.5)
project(rio_example)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)

add_executable(${PROJECT_NAME} main.cc)
target_include_directories(${PROJECT_NAME} PRIVATE 
					${PROJECT_SOURCE_DIR}
					${PROJECT_SOURCE_DIR}/../rio_lib
					${OpenCV_INCLUDE_DIRS}
					${EIGEN3_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} rio_lib ${OpenCV_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
# set_target_properties(${PROJECT_NAME} PROPERTIES EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)