add_subdirectory(qanviz)

message(STATUS "Build qanviz smoke... " ${SMOKE_ENABLED})

install( FILES qanviz_smoke.h DESTINATION include/smoke )
