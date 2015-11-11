cmake_minimum_required(VERSION 2.6)

## because CMakeForceCompiler is using deprecated behavior
cmake_policy(SET CMP0054 OLD)

set(smoke_srcs
    ${CMAKE_CURRENT_BINARY_DIR}/kdebindings/smoke/qanviz/smokedata.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/kdebindings/smoke/qanviz/x_1.cpp)
    
add_subdirectory(kdebindings)

set(CMAKE_INSTALL_PREFIX ..)

find_package(Qt4 4.4.0 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

## find_package() does not add the include directory for OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

include_directories( ${CMAKE_SOURCE_DIR}/kdebindings/smoke
                     ${CMAKE_SOURCE_DIR}/kdebindings/smoke/qanviz
                     ${R_INCLUDE_DIR} ${QTBASE_DIR}/local/include
                     ${QTBASE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
                     )

include(${QT_USE_FILE})

set(qtpaint_LIB_SRCS Layer.cpp Painter.cpp QtPainter.cpp imports.cpp
    OpenGLPainter.cpp RLayer.cpp init.cpp PainterWrappers.cpp
    ScenePainter.cpp paintUtils.cpp PlotView.cpp
    scene.cpp QtBasePainter.cpp convert.cpp TestWidget.cpp
    ${smoke_srcs})

QT4_WRAP_CPP (qtpaint_MOC_FILES Layer.hpp RLayer.hpp PlotView.hpp)

SET_SOURCE_FILES_PROPERTIES(${smoke_srcs} PROPERTIES GENERATED 1)
 
if(WIN32) # Toughest Win32 part: generating the defs file for the DLL
foreach(qtpaint_lib_src ${qtpaint_LIB_SRCS})
  get_filename_component(qtpaint_lib_src_basename ${qtpaint_lib_src} NAME)
  list(APPEND qtpaint_LIB_OBJS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/qtpaint.dir/${qtpaint_lib_src_basename}${CMAKE_CXX_OUTPUT_EXTENSION}")
endforeach(qtpaint_lib_src ${qtpaint_LIB_SRCS})
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtpaint.def 
                   COMMAND sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/mkdef.sh
                   DEPENDS ${qtpaint_LIB_OBJS}) 
SET(CMAKE_LINK_DEF_FILE_FLAG "qtpaint.def ") # space needs to be there
add_library(qtpaint SHARED qtpaint.def ${qtpaint_LIB_SRCS} ${qtpaint_MOC_FILES})
else(WIN32)
add_library(qtpaint SHARED ${qtpaint_LIB_SRCS} ${qtpaint_MOC_FILES})
endif(WIN32)

ADD_DEPENDENCIES(qtpaint generate_smoke)

target_link_libraries(qtpaint ${QT_LIBRARIES} ${R_LIBRARIES} ${OPENGL_LIBRARIES})

set_target_properties(qtpaint PROPERTIES PREFIX "")
if(NOT WIN32) # Need to force .so on Mac, but Windows needs to be .dll
set_target_properties(qtpaint PROPERTIES SUFFIX ".so")
install(TARGETS qtpaint LIBRARY DESTINATION src )
else(NOT WIN32)
install(TARGETS qtpaint RUNTIME DESTINATION src )
endif(NOT WIN32)

INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_CXX_COMPILER(${R_CXX} R_CXX)
