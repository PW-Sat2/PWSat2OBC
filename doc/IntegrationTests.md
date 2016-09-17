# Integration tests

## Things needed:
### Hardware
* EFM32GG Starter Kit
* Serial Port (for example  FT4232 Mini Module)
* DeviceMock:
    * Two when using both I2C buses
    * One when using single I2C bus (defualt)

### Software
* Toolchain (see [How to build](HowToBuild.md))
* Python 2.7 with pip

## Connections
* Starter Kit to PC (mini USB cable)
* If using FT4232, make sure required pins are connected (VBUS and VCC, V3V3 and VIO)
* DeviceMock to PC
* PC <-> Starter Kit:
 * RX from serial port to TX on starter kit (pin PD4)
 * TX from serial port to RX on starter kit (pin PD5)
 * RTS from serial port to RST on starter kit
 * GND from serial port to GND on starter kit
* Starter Kit <-> DeviceMock (System Bus)
 * GND
 * I2C bus: SDA (PC4), SCL (PC5)
* Starter Kit <-> DeviceMock (Payload Bus)
 * GND
 * I2C bus: SDA (PD6), SCL (PC7)

## Serial ports
Three (two for single bus mode) serial ports are used in integration tests:
 1. Terminal (called `OBC_COM`) used by tests to control behavior of OBC software 
 1. DeviceMock (called `SYS_BUS_COM`) used to simulate all devices that OBC needs to communicate with on System Bus (and Payload Bus in single bus mode)
 1. DeviceMock (called `PAYLOAD_BUS_COM`) used to simulate devices connected to Payload Bus (not used in single bus mode, as everything goes through system bus)

## Integration tests
Integration tests are written using Python and nosetests. In order to execute, information about serial ports is required. During creation of build system file `integration_tests/config.py` is created in build directory that contains all required information. That means that mention folder must be added to `PYTHONPATH` when executing tests. In PyCharm is can be achived by adding another source directory

Before running integration tests Starter Kit must be programmed with software being tested. 

## Running using CMake
Integration tests can be run from command line by calling `integration_tests` target is used build system. For Makefiles it will be:

`make integration_test`

That target depends on `pwsat` and `pwsat.flash` targets, so before running tests newest version of software will be built and programmed to Starter Kit. All configuration options will be passed, so no further configuration is needed.