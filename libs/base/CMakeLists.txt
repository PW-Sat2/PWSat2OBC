set(NAME base)

set(SOURCES
    reader.cpp
    writer.cpp
    ecc.cpp
    utils.cpp
    Include/base/reader.h
    Include/base/writer.h
    Include/system.h
    Include/base/os.h
    Include/base/ecc.h
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/base)
target_format_sources(${NAME} "${SOURCES}")
