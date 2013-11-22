cmake_minimum_required(VERSION 2.8.8)

project(kdebindings)

find_package(Qt5Core)

## FindQt4 does not add the include directory for OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

add_subdirectory(smoke)
