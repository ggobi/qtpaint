cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.4.0 COMPONENTS QtCore QtGui QtOpenGL REQUIRED )

## FindQt4 does not add the include directory for OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

include_directories( ../inst/local/include/smoke
                     ${R_INCLUDES} ${QTBASE_DIR}/local/include
                     ${QTBASE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

include(${QT_USE_FILE})

set(qtpaint_LIB_SRCS Layer.cpp Painter.cpp QtPainter.cpp imports.cpp
    OpenGLPainter.cpp RLayer.cpp init.cpp PainterWrappers.cpp
    ScenePainter.cpp paintUtils.cpp PlotView.cpp
    scene.cpp QtBasePainter.cpp convert.cpp TestWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp)
  
QT4_WRAP_CPP ( qtpaint_MOC_FILES Layer.hpp RLayer.hpp PlotView.hpp )

if(WIN32) # Toughest Win32 part: generating the defs file for the DLL
foreach(qtpaint_lib_src ${qtpaint_LIB_SRCS})
  list(APPEND qtpaint_LIB_OBJS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/qtpaint.dir/${qtpaint_lib_src}${CMAKE_CXX_OUTPUT_EXTENSION}")
endforeach(qtpaint_lib_src ${qtpaint_LIB_SRCS})
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtpaint.def 
                   COMMAND sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/mkdef.sh
                   DEPENDS ${qtpaint_LIB_OBJS}) 
SET(CMAKE_LINK_DEF_FILE_FLAG "qtpaint.def ") # space needs to be there
add_library(qtpaint SHARED qtpaint.def ${qtpaint_LIB_SRCS} ${qtpaint_MOC_FILES})
else(WIN32)
add_library(qtpaint SHARED ${qtpaint_LIB_SRCS} ${qtpaint_MOC_FILES})
endif(WIN32)

target_link_libraries(qtpaint ${QT_LIBRARIES} ${R_LIBRARIES})

set_target_properties(qtpaint PROPERTIES PREFIX "")
if(NOT WIN32) # Need to force .so on Mac, but Windows needs to be .dll
set_target_properties(qtpaint PROPERTIES SUFFIX ".so")
install(TARGETS qtpaint LIBRARY DESTINATION . )
else(NOT WIN32)
install(TARGETS qtpaint RUNTIME DESTINATION . )
endif(NOT WIN32)
