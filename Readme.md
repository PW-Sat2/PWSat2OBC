
# PW-Sat2 OBC Software

This repository contains source code of the PW-Sat2 On Board Computer (OBC) software.

The repository is divided into following parts:
  * \\doc - Documentation specific to the source code itself,
  * \\integration_tests - Contains sources of python end-to-end tests,
  * \\libs - Sources of the libraries used by the project,
    * \\libs\\drivers - Contains drivers for PW-Sat2 hardware,
    * \\libs\\external - Contains source code of all external libraries used by the project,
  * \\platforms - Contains modules that are specific to any of the platforms that PW-Sat2 project supports,
    * \\platforms\\DevBoard - Module that provides definitions for the EFM32GG-STK3700 development board used for development & integration testing,
  * \\src - Main OBC module,
  * \\unit_tests - unit test project.

## How to build

The list of the tools needed to build the OBC software and the instructions how to build it can be found [here](doc/HowToBuild.md).

# More about PW-Sat2

See [our website](https://pw-sat.pl) for details about PW-Sat2 CubeSat project.

PW-Sat2 is a student satellite project started in 2013 at Warsaw University of Technology by the Students Space Association members. Its main technical goal is to test new deorbit technology in form of a **large deorbit sail** whereas the project purpose is to educate a group of new space engineers. In February 2018 PW-Sat2 became fully integrated and was being prepared to the launch into orbit onboard Falcon 9 planned for the end of 2018.
