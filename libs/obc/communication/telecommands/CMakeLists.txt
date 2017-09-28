set(NAME obc_telecommands)

set(SOURCES
    comm.cpp
    ping.cpp
    file_system.cpp
    experiments.cpp
    program_upload.cpp
    time.cpp
    boot_settings.cpp
    antenna.cpp
    power.cpp
    fdir.cpp
    sail.cpp
    periodic_message.cpp
    flash.cpp
    i2c.cpp
    suns.cpp
    photo.cpp
    state.cpp
    compile_info.cpp
    eps.cpp
    adcs.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	telecommunication
	mission_antenna
	mission_comm
	mission_sail
	mission_time
	rtc
	fs
	logger
	exp_suns
	obc_experiments
	obc_fdir
	program_flash
	boot_settings
	power	
	suns
	payload
	gyro
	photo
	state
	version
	eps
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc/telecommands)
