add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp

    COMMAND ${QTBASE_DIR}/local/bin/smokegen
    ARGS -config "${QTBASE_DIR}/local/share/kdebindings/config.xml"
         -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml"
         -d "${QTBASE_DIR}/local/share/kdebindings/qtdefines"
         -I "${CMAKE_SOURCE_DIR}/../src"
         -- ${CMAKE_CURRENT_SOURCE_DIR}/qanviz_includes.h
    DEPENDS ${QTBASE_DIR}/local/share/kdebindings/config.xml
         ${QTBASE_DIR}/local/share/kdebindings/qtdefines
         smokeconfig.xml qanviz_includes.h
         ../../../src/Layer.hpp
         ../../../src/PlotView.hpp
         ../../../src/RLayer.hpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate_smoke ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
          ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp)
      
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
	${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp
        qanviz_includes.h
        DESTINATION ${CMAKE_SOURCE_DIR}/../src-build)
