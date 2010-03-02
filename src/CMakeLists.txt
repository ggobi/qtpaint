cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.4.0 COMPONENTS QtCore QtGui QtOpenGL REQUIRED )

include_directories( ../inst/kdebindings/smoke ../inst/kdebindings/smoke/qanviz
                     ${R_INCLUDES} ${QTBASE_DIR}/kdebindings/smoke
                     ${QTBASE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
                   )

include(${QT_USE_FILE})

set(qtpaint_LIB_SRCS Layer.cpp QtPainter.cpp imports.cpp
    OpenGLPainter.cpp RLayer.cpp init.cpp PainterWrappers.cpp
    ScenePainter.cpp paintUtils.cpp PlotView.cpp
    scene.cpp QtBasePainter.cpp convert.cpp smokedata.cpp x_1.cpp)

QT4_AUTOMOC ( ${qtpaint_LIB_SRCS} )

add_library(qtpaint SHARED ${qtpaint_LIB_SRCS})

target_link_libraries(qtpaint ${QT_LIBRARIES} ${R_LIBRARIES})

set_target_properties(qtpaint PROPERTIES PREFIX "")
set_target_properties(qtpaint PROPERTIES SUFFIX ".so")
