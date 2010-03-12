cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.4.0 COMPONENTS QtCore QtGui QtOpenGL REQUIRED )

include_directories( ../inst/local/include/smoke
                     ${R_INCLUDES} ${QTBASE_DIR}/local/include
                     ${QTBASE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

include(${QT_USE_FILE})

set(qtpaint_LIB_SRCS Layer.cpp QtPainter.cpp imports.cpp
    OpenGLPainter.cpp RLayer.cpp init.cpp PainterWrappers.cpp
    ScenePainter.cpp paintUtils.cpp PlotView.cpp
    scene.cpp QtBasePainter.cpp convert.cpp TestWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp)
  
QT4_WRAP_CPP ( qtpaint_MOC_FILES Layer.hpp RLayer.hpp PlotView.hpp )

add_library(qtpaint SHARED ${qtpaint_LIB_SRCS} ${qtpaint_MOC_FILES})

target_link_libraries(qtpaint ${QT_LIBRARIES} ${R_LIBRARIES})

set_target_properties(qtpaint PROPERTIES PREFIX "")
set_target_properties(qtpaint PROPERTIES SUFFIX ".so")

install(TARGETS qtpaint DESTINATION . )