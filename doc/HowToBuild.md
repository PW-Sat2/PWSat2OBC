# How to build

## Tools needed
Build process requires following tools:
* CMake
* GNU ARM Embedded toolchain (gcc-arm-none-eabi)
* Tools for build system used by CMake (e.g. GNU make)
* SEGGER J-Link (https://www.segger.com/jlink-software.html)
* QEmu with EFM32GG990 support (our fork, https://github.com/PW-Sat2/qemu/releases/tag/v2.4.1-efm)
* Doxygen
* Python2.7
* Clang (for `clang-format`)

### Ready-made pack for Windows
For Windows people ready-made pack with all necessary tools is located here: https://www.dropbox.com/s/zrf3c5uy9ujo687/PWSat-Toolchain.zip?dl=0
After unpacking, add `bin` directory to `PATH`.

### Unix - Ubuntu 15.10
Quick start packages:
* `apt-get install cmake`
* `wget https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2`
* `tar xf gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2`
* Only 32-bits build is delivered so follow http://askubuntu.com/questions/454253/how-to-run-32-bit-app-in-ubuntu-64-bit if you are using 64-bits Ubuntu
* `export ARM_TOOLCHAIN=(...)/gcc-arm-none-eabi-4_9-2015q3/bin/`

## Building
1. Clone repository (e.g. `C:\Work\src`)
1. Create directory `build` (`C:\Work\build`) as a sibling of source directory for CMake stuff
1. Go to `build` directory
1. Run `cmake -G <generator> <build-options> ..\src` (`<generator>` can be e.g. `Unix Makefiles` for Linux and `MinGW Makefiles` for Windows). For available build options see next section
1. Build using selected build system (`make` for Makefiles)

## Build options
There are number of build options available. All of them are passed to CMake by using format: `-D<option>=<value>`. Also, after creating Makefiles-based build it is possible to change them using `make edit_cache`.

Option | Default value | Description
------ | ------------- | -----------
`ARM_TOOLCHAIN`    | _None_               | Path to folder with toolchain binaries
`JLINK_PATH`       | _None_               | Path to folder with J-Link binaries
`CLANG_PATH`       | _None_               | Path to clang binaries
`TARGET_PLATFORM`  | `DevBoard`           | Platform that should be used as build target. For now only one is supported: `DevBoard`
`CMAKE_BUILD_TYPE` | `DEBUG`              | Type of build: `DEBUG` or `RELEASE`
`OBC_COM`          | _None_               | Serial port used to communicate with OBC terminal
`SYS_BUS_COM`      | _None_               | Serial port used to communicate with System Bus mock
`PAYLOAD_BUS_COM`  | _None_               | Serial port used to communicate with Payload Bus mock
`USE_SINGLE_BUS`   | 1                    | Set to 1 to enable only one I2C bus. Payload bus will be redirected to System Bus. Set to 0 to use both buses
`ENABLE_COVERAGE`  | 0                    | Set to 1 to enable code-coverage for unit tests

## Outputs
All output are located inside `build\DevBoard` directory. 
* `bin\pwsat` - ELF file with main project
* `bin\pwsat.hex` - HEX file with main project
* `bin\unit_tests` - ELF file with unit tests project

## Running Unit Tests 
Unit tests can be run on QEmu emulated device (EFM32GG990F1024). To run binaries call target `unit_tests.run`:
````
build> make unit_tests.run
[ 92%] Built target platform
[100%] Built target ut
[==========] Running 3 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 3 tests from MyTest
[ RUN      ] MyTest.MyName
Running my test
D:/tmp/testing/obc/test/test.cpp:9: Failure
Value of: 4
Expected: 2
[  FAILED  ] MyTest.MyName (0 ms)
[ RUN      ] MyTest.ThisOneWorks
Running my second test
[       OK ] MyTest.ThisOneWorks (0 ms)
[ RUN      ] MyTest.WritingFiles
[       OK ] MyTest.WritingFiles (0 ms)
[----------] 3 tests from MyTest (0 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 1 test case ran. (0 ms total)
[  PASSED  ] 2 tests.
[  FAILED  ] 1 test, listed below:
[  FAILED  ] MyTest.MyName

 1 FAILED TEST
[100%] Built target run_tests
````