set(NAME settings)

add_library(${NAME} INTERFACE)


target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/Include)

configure_file(settings.h.template ${CMAKE_CURRENT_BINARY_DIR}/Include/settings.h)