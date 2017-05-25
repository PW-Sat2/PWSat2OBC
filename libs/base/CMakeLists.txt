set(NAME base)

set(SOURCES
    reader.cpp
    writer.cpp
    ecc.cpp
    os_base.cpp
    crc.cpp
    BitWriter.cpp
    Include/base/reader.h
    Include/base/writer.h
    Include/system.h
    Include/base/os.h
    Include/base/ecc.h
    Include/base/crc.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    gsl
    freeRTOS
)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/base)
target_format_sources(${NAME} "${SOURCES}")
