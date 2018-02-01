# TuneOS
> A basic embedded operating system designed to support future LDG product development.

## Installing / Getting started

A quick introduction of the minimal setup you need to get a hello world up &
running.

```shell
commands here
```

Here you should say what actually happens when you execute the code above.

## Developing

### Built With
```
Windows 10 Creators Update

Microsoft Visual Studio Code

Microchip XC8 Compiler v 1.43
Microchip MPLABX IDE v 4.00
Microchip MPLAB Code Configurator v3.36
Microchip MCC Device Library for 8-bit PICs v1.45

CPPCHECK v1.80

CCSLoad v5.043

64 bit Cywin, including the following Cygwin apps:
make
git
open-ssh
```

### Setting up the development environment

Here's a brief intro about what a developer must do in order to start developing
the project further:

Installing the Microchip toolchain in the default location can cause problems because of spaces in paths/filenames.  It is recommended to install the Microchip tools to an alternate directory that does not contain spaces.  This is probably not an issue on Linux, but on Windows, one option is installing to "C:\Microchip\MPLABX" and "C:\Microchip\xc8".

This makes it easier to use some command line tools.  Invoking programs from the makefile is sometimes more complicated if the path includes a space.

Install the most recent version of Cygwin from https://cygwin.com/install.html



### Building

Build the hex file by compiling everything.
```shell
make all
```

Upload the hex to the PIC using the ICD-U80 programmer. (ccsload)
```shell
make upload
```

Upload the hex to the PIC using the PICkit3 programmer. (pk3cmd)
```shell
make upload-pk3
```

Build the hex and upload it to the PIC using the ICD-U80 in one command.
```shell
make all upload
```

Discard all old build files and recompile everything.
```shell
make clean
```

Discard all old build files, recompile everything, and upload it using the ICD-U80 programmer.
```shell
make clean all upload
```

Discard all old build files, recompile everything, and upload it using the ICD-U80 programmer.
```shell
make clean all upload-pk3
```

#### Shortcuts

Build the hex file.
```shell
make
```

Build and upload the hex file.
```shell
make upload
```

Rebuild everything and upload the hex file.
```shell
make clean upload
```

## Style guide

Explain your code style and show how to check it.