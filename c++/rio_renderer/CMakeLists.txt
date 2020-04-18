cmake_minimum_required(VERSION 3.5)
project(rio_renderer)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Eigen3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)

add_executable(${PROJECT_NAME} src/main.cc src/data.cc 
								src/util.cc src/renderer.cc src/json11.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include
					${EIGEN3_INCLUDE_DIR}
					${OPENGL_INCLUDE_DIR}
					${OpenCV_INCLUDE_DIRS}
					${GLFW_INCLUDE_DIRS}
					${GLEW_INCLUDE_PATH}
					${assimp_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${OPENGL_LIBRARIES}
									${GLFW_LIBRARIES} ${GLEW_LIBRARY} ${assimp_LIBRARIES})
