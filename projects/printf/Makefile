#
# Generic Makefile for MSP430-based projects
#
# based on code from theNoccyLabs
#
# (c) 2013, Broken Bit Labs, Michal Krombholz
# GPL v2
#
# Source files and target binary, define your source files here to have them
# compiled, and define the target basename. BUILD defines the default build
# type, as either "lib" or "bin". If bin, an .elf binary will be created as
# well as a .hex file, and if lib an .a library file will be created. To have
# the binary flashed on the device after build, use "bin prog".

# Source files, C, C++  and Assembly
SOURCEC   = main.c printf.c sput.c
SOURCECPP =
SOURCEASM =

# bin:binary file, lib:library, prog:program device
BUILD     = bin prog

# Target binary without extension.
TARGET    = code

# Include paths, f.ex. -I. -Isome/path
INCLUDES  =

# Libraries to link with, f.ex. -lcomm
LIBS      =
# Specify the lib directories, f.ex. -Llib -Lsome/path
LIBPATH   = 

# If you don't have common.mk for some reason you can manually download it from
# theNoccyLabs repository at http://sv.noccy.com/mspdev/skeleton/common.mk 
include ../../skeleton/common.mk
