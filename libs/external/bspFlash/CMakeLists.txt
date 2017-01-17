set(NAME bspFlash)

set(SOURCES
    lld.c
    asp/lld_bdsxxx_asp.c
    asp/lld_wsxxx_asp.c
    trace/trace.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-unused-parameter \
    -Wno-strict-prototypes \
    -Wno-unused-but-set-variable \
")