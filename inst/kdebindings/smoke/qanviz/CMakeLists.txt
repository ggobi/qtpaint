add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp

    COMMAND ${QTBASE_DIR}/kdebindings/generator/bin/generator
    ARGS -config "${QTBASE_DIR}/kdebindings/smoke/qt/config.xml"
         -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml"
         -d "${QTBASE_DIR}/kdebindings/smoke/qtdefines"
         -I "${CMAKE_SOURCE_DIR}/../../src"
         -- ${CMAKE_CURRENT_SOURCE_DIR}/qanviz_includes.h
    DEPENDS ${QTBASE_DIR}/kdebindings/smoke/qt/config.xml
         smokeconfig.xml qanviz_includes.h
         ${CMAKE_SOURCE_DIR}/../../src/Layer.hpp
         ${CMAKE_SOURCE_DIR}/../../src/PlotView.hpp
         ${CMAKE_SOURCE_DIR}/../../src/RLayer.hpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate_smoke ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
          ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp)
      
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp
        DESTINATION ${CMAKE_SOURCE_DIR}/../../src)
