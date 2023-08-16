file(GLOB _json_headers ${CMAKE_CURRENT_LIST_DIR}/*.h)

qas_wrap_cpp(_qasc_src ${_json_headers} TARGET ${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${_qasc_src})