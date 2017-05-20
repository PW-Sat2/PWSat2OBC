# Tools

## Unit test tool
This tool allows running single unit tests binary under QEmu and pass any parameters to it (listing tests, filtering tests, etc).

**Usage:**
	`<build>/tools/unit_tests.cmd <path to unit tests binary> <args...>`
	
## Integration tests tool
This tool allows running integration tests and passing any parameters (listing tests, filtering tests, etc). All necessary Python configuration is done inside script.

**Usage:**
	`<build>/tools/pytests.cmd <args...>`
	
## Boot entry upload tool
This tool allows automated uploading of program into boot table entry.

**Usage:**
	`<build>/tools/upload.cmd <port> <file> <index> <description>`

`port` - COM port for bootloader	
`file` - Binary file to upload (BIN format, not HEX)
`index` - Boot entry index (value from 1 to 7)
`description` - Boot entry description

## CRC calculation tool
This tool calculates XMODEM-like CRC value of given value.

**Usage:**
	`<build>/tools/crc.cmd <file>`
	
`file` - File in BIN format.

## Download safe mode program
This tool allows downloading safe mode from EEPROM using J-Link debug interface

**Usage:**
	`<build>/tools/download_safe_mode.cmd <file>`
	
`file` - Path to destination file

## Download program flash content
This tool allows downloading program flash content using J-Link debug interface

**Usage:**
	`<build>/tools/download_program_flash.cmd <file>`
	
`file` - Path to destination file