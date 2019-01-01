# **************************************************************************** #
# General Make configuration

# This suppresses make's command echoing. This suppression produces a cleaner output. 
# If you need to see the full commands being issued by make, comment this out.
MAKEFLAGS += -s

# Clear the default suffix list
.SUFFIXES:
# xc8 uses wierd intermediate files, NOT the standard .o file
.SUFFIXES: .c .h .p1 .as .obj .hex

# **************************************************************************** #
# Project stuff

# Project name 
PROJECT = AT-600ProII

# The PIC microcontroller used for this project
TARGET_DEVICE = 18f47k42

# **************************************************************************** #
# Primary targets

.PHONY: build create_directories lint program clean

# Running "make" should simply build the project
.DEFAULT: build

# Makefiles traditionally have an "all" target
all: build

# Target that actually builds the hex file
build: build_C89

# Target to clean up out files and directories
clean: clean_output_directories

# Target to make sure the hex is current, and then upload it to the target
upload: upload_ccs_linux

# **************************************************************************** #
# Directory stuff

# Source file directory
SRC_DIR = src

# Output directories
BUILD_DIR = build
OBJ_DIR = obj

# Target to make sure output directories exist
create_output_directories:
	mkdir -p ./$(BUILD_DIR)
	mkdir -p ./$(OBJ_DIR)

# Target to clean the output directories
clean_output_directories:
	-rm -r -f $(BUILD_DIR)/*
	-rm -r -f $(OBJ_DIR)/*

# **************************************************************************** #
# Hex file rules

# Get a space separated string of source files from the SRC_DIR, including subdirs
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) \
			 $(wildcard $(SRC_DIR)/**/*.c) \
			 $(wildcard $(SRC_DIR)/**/**/*.c)

# Get a space separated string of header files from the SRC_DIR, including subdirs
HEADER_FILES := $(wildcard $(SRC_DIR)/*.h) \
				$(wildcard $(SRC_DIR)/**/*.h) \
				$(wildcard $(SRC_DIR)/**/**/*.h)

# Full file name of the resulting hex file
PROJECT_HEX := $(BUILD_DIR)/$(PROJECT).hex

# .c -> .hex
$(PROJECT_HEX): $(SRC_FILES) $(HEADER_FILES)
	$(C89_COMMAND)

# **************************************************************************** #
# Commands and command variables

# This section contains drivers for both C89 and the new C99 versions of xc8.
# The C99 mode is too new to be used in production, but it produces different
# warnings than the C89 mode, and should be run occasionally as a kind of
# pseudo-linter.

# ---------------------------------------------------------------------------- #
# XC8 C89 mode
# This is the mode we want to use for production.

CC89 = xc8
# Specify what PIC we're using; is always required
C89FLAGS = --CHIP=$(TARGET_DEVICE)
# Tell xc8 where to put intermediate files(.pre, .p1, .d)
C89FLAGS += --OBJDIR=$(OBJ_DIR) 
# Tell xc8 where to put output files(.hex, .map, .cof, .as, etc)
C89FLAGS += -O$(BUILD_DIR)/$(PROJECT)
# Use hybrid-style stack. Additional fields are the desired size of, in order:
# stack size of (main code : low priority ISRs : high priority ISRs)
C89FLAGS += --STACK=hybrid:auto:auto:auto
# fill empty ROM with a repeating pattern
C89FLAGS += --FILL=0xffff
# Suppress most of xc8's command echos
C89FLAGS += -q
# Set the size of floating point types
C89FLAGS += --FLOAT=24 --DOUBLE=24

# There are several differences between the development hardware and the final
# product hardware. The code wraps these differences in #ifdefs so we don't
# accidentally forget to revert the changes.
C89FLAGS += -DDEVELOPMENT

# Optional compiler outputs and metrics

# Tell xc8 to report compilation times
C89FLAGS += --TIME
# Tell xc8 to generate the map file
C89FLAGS += -M$(BUILD_DIR)/$(PROJECT).map
# Tell xc8 to generate the assembly list file
# C89FLAGS += --ASMLIST

# Construct the final C89 XC8 command
C89_COMMAND := $(CC89) $(C89FLAGS) $(SRC_FILES)

# Target to compile in C89 mode
build_C89: $(PROJECT_HEX)


# ---------------------------------------------------------------------------- #
# XC8 C99 mode
# This mode is new as of XC8 v2.00, and is not mature yet. This mode is NOT
# ready for production work, but it features a new clang-based frontend that is
# useful an analysis tool.

CC99 = xc8-cc
# Specify what PIC we're using; is always required
C99FLAGS = -mcpu=$(TARGET_DEVICE)
# Set xc8 in C99 mode
C99FLAGS += -std=C99
# Tell xc8 to put all the output in the /build directory
C99FLAGS += -o $(BUILD_DIR)/$(PROJECT).hex
# Use the hybrid stack
C99FLAGS += -mstack=hybrid:auto:auto:auto

# There are several differences between the development hardware and the final
# product hardware. The code wraps these differences in #ifdefs so we don't
# accidentally forget to revert the changes.
C99FLAGS += -DDEVELOPMENT

# Construct the final C99 XC8 command
C99_COMMAND := $(CC99) $(C99FLAGS) $(SRC_FILES)

# Target to compile in C99 mode
build_C99:
	$(C99_COMMAND)

# **************************************************************************** #
# Linter(s)
# Linting and static analysis are very important tools for maintaining code
# quality. If we have access to any of these tools that work on our target code,
# we should run them automatically on every build.

# ---------------------------------------------------------------------------- #
# Cppcheck is a free C/C++ static analysis tool.
# http://cppcheck.sourceforge.net/

LINT = cppcheck
# Tell cppcheck which C standard to check against
LINTFLAGS = --std=c89
# Tell cppcheck to run all test categories 
LINTFLAGS += --enable=all
# Tell cppcheck where xc8 headers live
# TODO: file paths are not portable
LINTFLAGS += -I C:\Microchip\xc8\v2.00\pic\include
LINTFLAGS += -I C:\Microchip\xc8\v2.00\pic\include\c90
# Allow that Cppcheck reports even though the analysis is inconclusive.
LINTFLAGS += --inconclusive
# Force checking of all #ifdef configurations. This takes significantly longer!
LINTFLAGS += --force
# Enable inline suppressions in the source 
LINTFLAGS += --inline-suppr
# 8bit AVR is the most similar platform to PIC18
LINTFLAGS += --platform=avr8
# Do not show progress reports
LINTFLAGS += -q

# Specify the number of threads to use
# Enabling multithreading disables unusedFunction check.
LINTFLAGS += -j 6

# Check cppcheck configuration. The normal code analysis is disabled.
# This option can be used to confirm that cppcheck sees every header file.
# CPPCFLAGS += --check-config

# Run all linters 
lint:
	$(LINT) $(LINTFLAGS) $(SRC_FILES)

# **************************************************************************** #
# Automated programming targets

# These targets are used by the "make upload" command to build the project and
# automatically upload the hex file to the destination device

# ---------------------------------------------------------------------------- #
# Microchip Pickit3 USB programmer
# Manual can be found at:
# C:/Microchip/MPLABX/v5.00/docs/Readme for PK3CMD.htm
# or the equivalent directory on your system.

PK3 = pk3cmd
# Specify what PIC we're using; is always required
PK3FLAGS = -P$(TARGET_DEVICE) 
# Specify the hex file to be programmed
PK3FLAGS += -F$(PROJECT_HEX)
# Specify the memory regions to be programmed ("-M<region>")
# No <region> argument means to program the entire device
PK3FLAGS += -M 
# Release the device from RESET after programming
PK3FLAGS += -L 
# Power the device from the PICkit3 using the specified voltage
# PK3FLAGS += -V5
# Verify the device against the hex file
# PK3FLAGS += -Y

# Target for uploading with Microchip Pickit3
upload_pk3: build
	$(PK3) $(PK3FLAGS)

# pk3cmd sometimes creates a bunch of files we don't want
PK3_GARBAGE = log.0 log.1 log.2 MPLABXLog.xml MPLABXLog.xml.lck

# Clean pk3cmd's stupid sometimes-output
clean_pk3:
	-rm $(PK3_GARBAGE)

# ---------------------------------------------------------------------------- #
# Microchip Pickit4 USB programmer, controlled by ipecmd.exe
# Manual can be found at:
# C:/Microchip/MPLABX/v5.00/docs/Readme for MPLAB IPE.htm
# or the equivalent directory on your system.

# chip -P<Part name>
PK4 = ipecmd
# specify Pickit4
PK4FLAGS = -TPPK4
# select the project PIC
PK4FLAGS += -P$(TARGET_DEVICE)
# Specify the hex file to be programmed
PK4FLAGS += -F$(PROJECT_HEX)
# -M program the entire device
PK4FLAGS += -M
# -Y verify the entire device
# PK4FLAGS += -Y
# command flag used to specify input hex file
PK4HEXFLAG = -F

# Target for uploading with Microchip Pickit3
upload_pk4: build
	$(PK4) $(PK4FLAGS)

# pk4cmd sometimes creates a bunch of files we don't want
PK4_GARBAGE = log.0 log.1 log.2 MPLABXLog.xml MPLABXLog.xml.lck

# Clean pk4cmd's stupid sometimes-output
clean_pk4:
	-rm $(PK4_GARBAGE)

# ---------------------------------------------------------------------------- #
# CCS ICD-U80 USB programmer on Windows

WIN_CCSLOAD = ccsloader
# Specify what PIC we're using; is always required
WIN_CCSLOADFLAGS = -DEVICE=PIC$(TARGET_DEVICE)
# Program all the device's memory regions
WIN_CCSLOADFLAGS += -AREAS=ALL 
# Don't supply power to the target
WIN_CCSLOADFLAGS += -POWER=TARGET
# Specify the hex file to be programmed
WIN_CCSLOADFLAGS += -WRITE=$(PROJECT_HEX)


# Target for uploading with CCS ICD-U80
upload_ccs_win: build
	$(WIN_CCSLOAD) $(WIN_CCSLOADFLAGS)


# ---------------------------------------------------------------------------- #
# CCS ICD-U80 USB programmer on Linux

LINUX_CCSLOAD = /opt/picc/ccsloader
# Specify what PIC we're using; is always required
# Also specify the serial device.
# TODO: Why the fuck does -port not work when it's on its own line!?!?!
LINUX_CCSLOADFLAGS = -DEVICE=PIC$(TARGET_DEVICE) -port=COM7
# Program all the device's memory regions
LINUX_CCSLOADFLAGS += -AREAS=ALL 
# Don't supply power to the target
LINUX_CCSLOADFLAGS += -POWER=TARGET
# Specify the hex file to be programmed
LINUX_CCSLOADFLAGS += -WRITE=$(PROJECT_HEX)

# Target for uploading with CCS ICD-U80
upload_ccs_linux: build
	$(LINUX_CCSLOAD) $(LINUX_CCSLOADFLAGS)

# ---------------------------------------------------------------------------- #
# MeLabs U2 USB programmer

# TODO: add melabs command line driver here
MELABS = meprog
# TODO: add melabs command line flags here
MELABSFLAGS = empty

# Target for uploading with MeLabs U2
upload_melabs: build
	$(MELABS) $(MELABSFLAGS)

# DO NOT DELETE
