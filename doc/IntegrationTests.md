# Integration tests

## Things needed:
### Hardware
* OBC EM 2.02
* Serial Port (for example FT4232 Mini Module)
* DeviceMock (STM version)

### Software
* Toolchain (see [How to build](HowToBuild.md))
* Python 2.7 with pip
* (Windows only) Visual C++ for Python 2.7 (http://aka.ms/vcpython27)

## Connections
* Starter Kit to PC (mini USB cable)
* If using FT4232, make sure required pins are connected (VBUS and VCC, V3V3 and VIO)
* DeviceMock to PC
* PC <-> OBC EM:
 * RX from serial port to TX on EM
 * TX from serial port to RX on EM
 * RTS from GPIO serial port to RST on EM
 * GND from serial port to GND on EM
* Starter Kit <-> STM DeviceMock
 * GND
 * I2C System Bus
 	* EM: SDA (PC6), SCL (PC7)
	* STM: SDA (PB9), SCL (PB8)
 * I2C Payload Bus
 	* EM: SDA (PC4), SCL (PC5)
	* STM: SDA (PB11), SCL (PB10)
 * Payload Busy/Interrupt Pin
 	* STM: PA8 <-> STK: PE0

STM pinout can be found here: [http://www.st.com/content/ccc/resource/technical/document/user_manual/98/2e/fa/4b/e0/82/43/b7/DM00105823.pdf/files/DM00105823.pdf/jcr:content/translations/en.DM00105823.pdf](http://www.st.com/content/ccc/resource/technical/document/user_manual/98/2e/fa/4b/e0/82/43/b7/DM00105823.pdf/files/DM00105823.pdf/jcr:content/translations/en.DM00105823.pdf), page 28, figure 10.

## Serial ports
Four (three for single bus mode) serial ports are used in integration tests:
 1. Terminal (called `OBC_COM`) used by tests to control behavior of OBC software 
 1. GPIO (called `GPIO_COM`) used by tests to reset OBC and request cleaning state at startup
 1. STM DeviceMock (called `MOCK_COM`) used to simulate all devices that OBC needs to communicate with on System and Payload Bus

## Integration tests
Integration tests are written using Python and nosetests. In order to execute, information about serial ports is required. During creation of build system file `integration_tests/config.py` is created in build directory that contains all required information. That means that mention folder must be added to `PYTHONPATH` when executing tests. In PyCharm is can be achived by adding another source directory

Before running integration tests Starter Kit must be programmed with software being tested. 

## Running using CMake
Integration tests can be run from command line by calling `integration_tests` target is used build system. For Makefiles it will be:

`make integration_test`

That target depends on `pwsat` and `pwsat.flash` targets, so before running tests newest version of software will be built and programmed to EM. All configuration options will be passed, so no further configuration is needed.

# pyconsole
Ipython console can be invoked for semi-manual testing. It runs device mock, allowing user to interact with OBC.

## Requirements:
* wx [https://www.wxpython.org/download.php#msw](https://www.wxpython.org/download.php#msw)
* ipython (install via pip)

## Example:
	make pyconsole
	from obc import AntennaChannel
	m = system.obc.antenna_get_status(AntennaChannel.Primary)
	m.DeploymentState
