set(NAME obc_hardware)

set(SOURCES
    hardware.cpp
    Include/obc/PersistentStorageAccess.hpp
    PersistentStorageAccess.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	gpio
	i2c
	spi
	power
	burtc
	mcu_temp
	obc_state
	gyro
	eps
	program_flash
	antenna
	imtq
	rtc
	uart
	comm
	flash_s29jl
	msc
	payload
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc)
