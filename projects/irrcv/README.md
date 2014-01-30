MSP430 IR receiver
====================

The IR (infra-red as in TV remote) receiver decodes a "PROTOCOL TracerJet Heli" RC codes.

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

    * identify - Identify the attached microcontroller using mspdebug. add export MCU=msp430xxxxx (whatever the identify return) to bypass id on every build

            * debugs - launch mspdebug

            * debug - laung msp430gdb and connect to the mspdebug


// "Protocol TracerJet Heli" RC remote protocol description.
//
// The remote pulses IR (@38kHz carrier) for a variable time of 400us
// the time between IR bursts is used to mean one of 3 SYMBOLs:
// START, LONG and SHORT.
// The command consist of sequence of 500us burst then START then 32 bits of data.
// the gap between IR bursts is 400us
// the gap between commands is about ~170ms
//
// start symbol + 32bit, IR burst off time is 400us
// symbol times from edge of IR to next edge of IR
// start    = 2000us
// long(1)  = 1200us
// short(0) =  800us
//
// Detection Algorithm
//
// The symbols are timed from falling edge of IR to the next fallling edge of IR (ie end of burst).
// However since the IR detector uses "0" level as "active" (ie IR present)
// the pin interrupts as active on "rising" edge of detector signal.
// The algoritm looks for the START symbol, and then expects the bits to follow.
// The first 500us burst is only used to time next symbol (ie START symbol).
// However, the alg is not perfect as it hinges on receiving and recognizing correct START symbol.
// A possibility exist that wrapping of time ticks would produce a delta time matching start by chance.
// This is however rather unlikely (more unlikely when timer wraparound is rare/longer).
// Also, the alg has no build in timeout and when not all bits are received in one IR command
// (due to interference) it will continue to read bits of next one and eventually err (too many bits).
// Hence it will faile receiving both commands (bad one and next potentially good one).
//
