set(NAME exp_pld)

set(SOURCES
    payload_exp.cpp
    PayloadExperimentTelemetryProvider.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	experiments
	fs
	payload
	time
	exp_fs
	exp_camera
	power
	suns
	telemetry
	mcu_temp
	obc_fdir
	photo
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/experiment/payload)
