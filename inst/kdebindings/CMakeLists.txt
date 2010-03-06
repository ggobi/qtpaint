cmake_minimum_required(VERSION 2.6)

project(kdebindings)

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

include(${QT_USE_FILE})

add_subdirectory(smoke)