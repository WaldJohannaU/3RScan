cmake_minimum_required(VERSION 3.5)
project(rio) 

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

add_subdirectory(src/rio_lib)
add_subdirectory(src/example)