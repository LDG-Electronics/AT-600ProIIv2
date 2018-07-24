# This suppresses make's command echoing. This suppression produces a cleaner output. 
# If you need to see the full commands being issued by make, comment this out.
MAKEFLAGS += -s

.DEFAULT: all
all: create_directories build

# Clear the default suffix list, the XC8 Compiler does not use a .o file type
.SUFFIXES:
.SUFFIXES: .c .h .p1 .as .obj .hex

# ---------------------------------------------------------------------------- #

# Directories revelant to the build process
BUILD_DIR = build
SRC_DIR = src
OBJ_DIR = obj

# The name of the project
PROJECT = AT-600ProII
PROJECT_HEX = $(BUILD_DIR)/$(PROJECT).hex

# The PIC microcontroller used for this project
TARGET_DEVICE = 18f46k42

# ---------------------------------------------------------------------------- #
# Commands and command variables

# Compiler - This project uses Microchip XC8
CC = xc8 
# --CHIP= is always required
CFLAGS = --CHIP=$(TARGET_DEVICE) -D_$(TARGET_DEVICE)
# -D defines the symbol after it as a preprocessor macro
CFLAGS += -D_$(TARGET_DEVICE)
# -q suppresses most of xc8's command echos
CFLAGS += -q
# --OBJDIR tells xc8 where to put intermediate files(.pre, .p1, .d)
CFLAGS += --OBJDIR=$(OBJ_DIR) 
# -O tells xc8 where to put output files(.hex, .map, .cof, .as, etc)
CFLAGS += -O$(BUILD_DIR)/$(PROJECT)
# -M tells the compiler to generate the map file
# CFLAGS += -M$(BUILD_DIR)/$(PROJECT)
# --ASMLIST tells the compiler to generate the assembly list file
# CFLAGS += --ASMLIST
# These set the size of floating point types to the larger 32bit settings
CFLAGS += --FLOAT=24 --DOUBLE=32
# Tells the compiler to report compilation times
CFLAGS += --TIME
# Fill unused space in the hex 
CFLAGS += --FILL=0xDEADC0DE@unused
# Use hybrid-style stack. Additional fields are the desired size of, in order:
# stack size of (main code : low priority ISRs : high priority ISRs)
CFLAGS += --STACK=hybrid:auto:auto:auto

# TODO: Fix the Vector Table!
# This flag suppresses about 80 warning messages complaining that the Vector Tables
# for the interrupt handler are empty.
# WARNING: Do not suppress compiler warnings during normal development!
CFLAGS += --MSGDISABLE=2020:off
# Program each unassigned vector with the address of a RESET instruction
CFLAGS += --UNDEFINTS=RESET

# Linter(s)
# LINT1 is cppcheck, a free C/C++ static analysis tool.
LINT1 = cppcheck
#LINT1FLAGS = -q
LINT1FLAGS = --check-config
LINT1FLAGS += --enable=all
LINT1FLAGS += --inline-suppr
LINT1FLAGS += --language=c
LINT1FLAGS += --platform=avr8

# Microchip Pickit3 USB programmer
# chip -P<Part name>
# -M Erase and program the entire device
PK3 = pk3cmd
PK3FLAGS = -P$(TARGET_DEVICE) 
PK3FLAGS += -M 
PK3FLAGS += -L 
# PK3FLAGS += -V5
PK3_GARBAGE = log.0 log.1 log.2 MPLABXLog.xml MPLABXLog.xml.lck

# CCS ICD-U80 USB programmer
CCSLOAD = ccsload
CCSLOADFLAGS = -AREAS=ALL 
CCSLOADFLAGS += -VERBOSE 
CCSLOADFLAGS += -DEVICE=PIC18F46K42 
CCSLOADFLAGS += -POWER=TARGET
CCSLOADFLAGS += -WRITE=$(PROJECT_HEX)

# MeLabs U2 USB programmer
# TODO: add melabs command line driver here
MELABS = meprog

# ---------------------------------------------------------------------------- #
# Puttin' stuff together

# Get a space separated string of .c and .h files from the SRC_DIR, including subdirs
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/**/**/*.c)

# ---------------------------------------------------------------------------- #
# Rules

# .c -> .hex
$(PROJECT_HEX): $(SRC_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES)

# ---------------------------------------------------------------------------- #
# Targets

# Declare all targets as phony, 
.PHONY: build create_directories lint program clean

build: $(PROJECT_HEX)

# Creates /(BUILD_DIR), if it doesn't already exist
create_directories:
	mkdir -p ./$(BUILD_DIR)
	mkdir -p ./$(OBJ_DIR)

lint:
	$(LINT1) $(LINT1FLAGS) $(SRC_FILES)

upload-pk3: $(PROJECT_HEX)
	$(PK3) $(PK3FLAGS) -F$(PROJECT_HEX)
	# pk3cmd.exe sometimes produces a bunch of files that I don't want.
	# There's no flag to control this output, so we're just going to 
	# remove them automatically.
	-rm $(PK3_GARBAGE)

upload-ccs: create_directories $(PROJECT_HEX)
	$(CCSLOAD) $(CCSLOADFLAGS)

upload-melabs:

upload:	upload-ccs

clean:
	-rm -r -f $(BUILD_DIR)/*
	-rm -r -f $(OBJ_DIR)/*
	-rm -f $(PROGRAM_GARBAGE)
	-rm -f $(PK3_GARBAGE)

# DO NOT DELETE
