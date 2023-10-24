target_sources_ifdef(CONFIG_ZERV app PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/sub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/zerv_internal.c
)

if(DEFINED CONFIG_ZERV)
  target_include_directories(app PRIVATE .)
endif()