cmake_minimum_required(VERSION 3.5)
project(rio_lib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../lib)

add_library(${PROJECT_NAME}
    rio_lib/lib.h
    rio_lib/rio.h rio.cc
    rio_lib/data.h data.cc
    rio_lib/sequence.h sequence.cc 
    rio_lib/types.h types.cc
    rio_lib/utils.h
    rio_lib/frame_config.h
    rio_lib/data_config.h
    rio_lib/rio_config.h
    third_party/json11.cpp
    third_party/json11.hpp
    third_party/tiny_obj_loader.h
    third_party/tinyply.cpp
    third_party/tinyply.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}
                           ${OpenCV_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
# set_target_properties(${PROJECT_NAME} PROPERTIES EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../lib)