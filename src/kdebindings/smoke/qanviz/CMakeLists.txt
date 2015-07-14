add_custom_command(OUTPUT ${smoke_srcs}
    COMMAND ${QTBASE_DIR}/local/bin/smokegen
    ARGS -config "${QTBASE_DIR}/local/share/kdebindings/config.xml"
         -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml"
         -d "${QTBASE_DIR}/local/share/kdebindings/qtdefines"
         -I "${CMAKE_SOURCE_DIR}"
         -- ${CMAKE_CURRENT_SOURCE_DIR}/qanviz_includes.h
    DEPENDS ${QTBASE_DIR}/local/share/kdebindings/config.xml
         ${QTBASE_DIR}/local/share/kdebindings/qtdefines
         smokeconfig.xml qanviz_includes.h
         ${CMAKE_SOURCE_DIR}/Layer.hpp
         ${CMAKE_SOURCE_DIR}/PlotView.hpp
         ${CMAKE_SOURCE_DIR}/RLayer.hpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate_smoke DEPENDS ${smoke_srcs})
