cmake_minimum_required(VERSION 2.6)

project(kdebindings)

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

## FindQt4 does not add the include directory for OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

include(${QT_USE_FILE})

add_subdirectory(smoke)