cmake_minimum_required (VERSION 2.6)
project (CSKETCHP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin/")

ADD_DEFINITIONS(-std=c++11)

set(APP_SOURCES
      ./main.cpp
)
      
message(STATUS "source files: ${APP_SOURCES}")
add_executable(csketchp ${APP_SOURCES})
target_link_libraries (csketchp sfml-window)
target_link_libraries (csketchp sfml-system)
target_link_libraries (csketchp sfml-graphics)
