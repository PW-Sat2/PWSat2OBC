set(NAME obc_experiments)

set(SOURCES
    experiments.cpp
)

 add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    fs
    experiments
    exp_fibo
    exp_adcs
    exp_leop
    exp_suns
    exp_flash
    exp_pld
    exp_radfet
    exp_sail
    exp_sads
    exp_program
    payload
    power
    photo
)


target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc)
