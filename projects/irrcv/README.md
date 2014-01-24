MSP430 Servos Server
====================

Simple driver for multiple servos. 
The servo pulses are generated in the interrupt. The update rate can be in the range of 10-20ms (50-100us).
The isr generates a puls of 1000-2000us and is driver by 0-255 x 10us variable in the array (one entry per particular servo).

TOOD:
+ multiple servos control (configurable)
+ decide if ISR fires with "update period", dynaic adjust (ie next pin update event) or all pulses are generated "polling style"
+ accept control input over serial port 

Requirements
------------

To compile this project you need a MSP430-based device and programmer. It is
designed to be used with the Launchpad, but the binaries should be universal
and can be programmed onto any compatible device with any programmer.

 * mspdebug is needed to identify the device as well as program it
 * msp430-gcc is needed to compile the project

The Makefile
------------

The makefile is based on the mspdev skeleton makefile and thus prvides the
following actions:

 * all - Build all the source files, link the binary and then call on mspdebug
   to program the device. This is the default action, and thus just calling
   make will instantly deploy the project on the microcontroller, just like
   with the picdev makefile.

 * bin - Build all the source files and link the binary in elf and hex format.
   No programming is done here.

 * prog - Program the device with the binary.

 * clean - Clean the build environment and remove all temporary files.

 * listing - Generate assembly source listings from the .c files.

 * identify - Identify the attached microcontroller using mspdebug.


