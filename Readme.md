
## PW-Sat 2

This repository contains source code of the PW-Sat 2 On Board Computer (OBC) software.

The repository is divided into following parts:
  * \\doc - Documentation specific to the source code itself,
  * \\integration_tests - Contains sources of python end-to-end tests,
  * \\libs - sources of the libraries used by the project,
    * \\libs\\drivers - Contains drivers for PW-Sat 2 hardware,
    * \\libs\\external - Contains source code of all external libraries used by the project,
  * \\platforms - Contains modules that are specific to any of the supported platforms that PW-Sat 2 project supports,
    * \\platforms\\DevBoard - Module that provides definitions for the EFM32GG-STK3700 used for development & integration testing,
  * \\src - Main OBC module,
  * \\unit_tests - unit test project.

## How to build

The list of the tools needed to build the OBC software and the instructions how to build it can be found [here](https://github.com/PW-Sat2/PWSat2OBC/wiki/How-to-build).

