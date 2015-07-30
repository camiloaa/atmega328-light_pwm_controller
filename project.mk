############################################################################################################################################
#
#     Content: Project configuration file for IGM project
#
#     Author: Mattias Holmqvist, Cybercom Sweden, +46 706 79 31 91
#
#     Description: This file contains the configuration settings and special make rules to build the 
#
############################################################################################################################################
# specify where to store object files
OBJDIR = obj
# specify where to store the linked out file
# TARGET_DIR = bin
# specify the name of the target
TARGET_NAME = light
# specify the final output directory
BIN = $(OBJDIR)/$(TARGET_NAME).elf
HEXREC = $(OBJDIR)/$(TARGET_NAME).hex

############################################################################################################################################
# sources from sig config generation, add only the source files
C_SOURCES = light.c tables.c timer.c uart.c

SOURCES = $(C_SOURCES) $(CPP_SOURCES) $(ASM_SOURCES)


############################################################################################################################################
# specify paths to the sources
SOURCEDIRS = src

# specify the paths to the include files
INCLUDEDIRS = include

# specify the linker file name
# LDSCRIPT = LPC17xx.ld

# specify the paths to library files
LIBPATH = 

# specify the library files to include in the build
LIBS = 

# specify project DEFINES
CFLAGS += -D__AVR_ATmega328p__
# common include file with standard types, macros and inline functions for the whole project
# CFLAGS += -include types.h -include arm_comm.h

############################################################################################################################################
# target for all
all: image hex

image: $(BIN)

hex: $(HEXREC)
