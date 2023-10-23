target_sources_ifdef(CONFIG_PUBSUB app PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/sub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/zit_serv_internal.c
)

if(DEFINED CONFIG_PUBSUB)
  target_include_directories(app PRIVATE .)
endif()