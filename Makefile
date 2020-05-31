# **************************************************************************** #
# General Make configuration

# This suppresses make's command echoing. This suppression produces a cleaner output. 
# If you need to see the full commands being issued by make, comment this out.
MAKEFLAGS += -s
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --warn-undefined-variables

# select shell 
ifeq ($(OS),Windows_NT)
SHELL = cmd
else
SHELL = bash
endif

# **************************************************************************** #
# Primary targets
	
# Build the project
compile: venv project_hex

# Upload the hex file to the target device
upload: venv project_hex 
	$(VENV_PYTHON) scripts/upload.py

# Remove the compiler outputs
clean:
	rm -rf ./$(BUILD_DIR)/*
	rm -rf ./$(OBJ_DIR)/*

# Run cog to perform in-line code generation
cog: venv
	$(VENV_PYTHON) -m cogapp -Iscripts -p "import cogutils as utils" @cogfiles.txt 
	rm -rf ./src/__pycache__
	
# **************************************************************************** #
# python venv settings

VENVDIR := .venv

ifeq ($(OS),Windows_NT)
	VENV := $(VENVDIR)\Scripts
	PYTHON := python
	VENV_PYTHON := $(VENV)\$(PYTHON)
	VENV_PIP := $(VENV)\pip
	VENV_MARKER := $(VENV)\.initialized-with-Makefile.venv
else
	VENV := $(VENVDIR)/bin
	PYTHON := python3
	VENV_PYTHON := $(VENV)/$(PYTHON)
	VENV_PIP := $(VENV)/pip3
	VENV_MARKER := $(VENV)/.initialized-with-Makefile.venv
endif

# Use this as an "order independent target" and Make will ensure the venv exists
.PHONY: venv
venv: $(VENV_MARKER)

# Creates the venv if it doesn't exist at VENVDIR
$(VENV):
	$(PYTHON) -m venv $(VENVDIR)

# Updates the venv according to REQUIREMENTS
$(VENV_MARKER): | $(VENV)
	$(VENV_PIP) install -r requirements.txt
	touch $@

# deletes the venv
clean_venv:
	rm -rf ./.venv

# deletes the venv and rebuilds it
reset_venv: clean_venv venv

# **************************************************************************** #
# Project stuff

# Project name 
PROJECT := $(shell $(PYTHON) scripts/load_vars.py name)

# Source file directory
SRC_DIR := $(shell $(PYTHON) scripts/load_vars.py src_dir)

# Output directories
BUILD_DIR := $(shell $(PYTHON) scripts/load_vars.py build_dir)
OBJ_DIR := $(shell $(PYTHON) scripts/load_vars.py obj_dir)

# Every C source file and header
PROJECT_FILES := $(shell $(PYTHON) scripts/find_source_files.py $(SRC_DIR))

# The project config file
PROJECT_FILES += project.yaml

# The build script
PROJECT_FILES += scripts/build.py

# Make sure the hex gets rebuilt if any of its deps change
$(BUILD_DIR)/$(PROJECT).hex: $(PROJECT_FILES) | obj_dir build_dir
	$(VENV_PYTHON) scripts/build.py

project_hex: $(BUILD_DIR)/$(PROJECT).hex 

# **************************************************************************** #
# creates output directories

ifeq ($(OS),Windows_NT)
MK_DIR := -md
else
MK_DIR := mkdir -p
endif

obj_dir: $(OBJ_DIR)
$(OBJ_DIR):
	$(MK_DIR) $(OBJ_DIR)

build_dir: $(BUILD_DIR)
$(BUILD_DIR):
	$(MK_DIR) $(BUILD_DIR)

# DO NOT DELETE