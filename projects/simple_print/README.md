MSP430 Simple Serial UART
====================

Simple serial output for MCUs without UART. 
This is TX (output only).
Does not use any TIMERs - the timing is purly through busy loop.

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


