# Runlevels

## Need
Testing the OBC software requires diffrent 'modes' of running OBC - fully autonomous (like on the orbit) or manual mode that allows uninterrupted interaction with OBC itself or other subsystems by terminal commands. 

These requirements can be satisifed by mechanism similar to **runlevels** from Linux systems - it is possible to init system up to specific runlevels with more and more fully running operating system. Moreover it is always possible to switch to higher runlevel without restarting whole system but switching to lower level requires system restart.

## OBC runlevels
For OBC purposes three runlevels have be identified:
1. Bootloader
2. Manual control mode
3. Fully autonomous mode 

### Runlevel: Bootloader 
This runlevel stops on bootloader stage - it allows uploading new program and switching currently selected boot index. 

### Runlevel: Manual control
It this mode OBC software is running and all bus subsystem are initialized. However none of the automatic behaviors are running (mission loop, frame receiver, etc are halted). This runlevel allows using terminal to access other subsystem or test specific pieces of functionality without interruption from other elements and by using manual control (like running mission loop step-by-step).

### Runlevel: Fully autonomous
This is default runlevel if not specified otherwise during boot sequence. All subsystems are initialized and automatic actions are taken as planned (mission loop is running, frames are being received, etc).

## Implementation
Runlevel 1 is implemented by bootloader. Switching to higher runlevel is achieved by loading OBC program and running it.
Runlevels 2 and 3 are manifested in code by methods on @ref OBC class. Each of them is responsible for enabling all required services. Additionaly specific services/behaviours can be enebled by terminal command. 

## How-to: staying on bootloader runlevel
During startup bootloader outputs character '&' on UART than waits for T<sub>boot\_select</sub> milliseconds to receive character 'S'. If it is not received, normal boot procedure is performed. Otherwise OBC responds with 'O' and enters runlevel 0.

## How-to: running OBC in runlevel 2
OBC runlevel is selected in bootloader, so procedure to enter runlevel to is as follows:
1. Enter runlevel 1 
2. Select boot option: external flash or MCU flash
3. Select runlevel: 2
4. OBC program will be loaded and will stay in runlevel 2 waiting for commands.