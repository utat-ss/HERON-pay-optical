# Reference: http://www.atmel.com/webdoc/avrlibcreferencemanual/group__demo__project_1demo_project_compile.html

# Don't change these
# AVR-GCC compiler
CC = avr-gcc
# Compiler flags
CFLAGS = -Wall -std=gnu99 -Wl,-u,vfprintf -g -mmcu=atmega8a -Os -mcall-prologues
# Includes (header files)
INCLUDES = -I./lib-common-ported/include/
# Programmer
PGMR = stk500
# Microcontroller
MCU = m8
# Build directory
DIR = build
# Program name
PROG = pay_optical

# Libraries from lib-common to link
# May need to change this line
LIB = -L./lib-common-ported/lib -lpex -luart -lspi -li2c -lstack -lqueue -lutilities -lprintf_flt -lm

# Detect operating system - based on https://gist.github.com/sighingnow/deee806603ec9274fd47

# One of these flags will be set to true based on the operating system
WINDOWS := false
MAC_OS := false
LINUX := false

ifeq ($(OS),Windows_NT)
	WINDOWS := true
else
	# Unix - get the operating system
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		MAC_OS := true
	endif
	ifeq ($(UNAME_S),Linux)
		LINUX := true
	endif
endif

# PORT - Computer port that the programmer is connected to
# Try to automatically detect the port
ifeq ($(WINDOWS), true)
	# higher number
	PORT = $(shell powershell "[System.IO.Ports.SerialPort]::getportnames() | sort | select -First 2 | select -Last 1")
endif
ifeq ($(MAC_OS), true)
	# lower number
	PORT = $(shell find /dev -name 'tty.usbmodem[0-9]*' | sort | head -n1)
endif
ifeq ($(LINUX), true)
	# lower number
	# TODO - test this
	PORT = $(shell find /dev -name 'ttyS[0-9]*' | sort | head -n1)
endif

# If automatic port detection fails,
# uncomment one of these lines and change it to set the port manually
# PORT = COM16						# Windows
# PORT = /dev/tty.usbmodem00208212	# macOS
# PORT = /dev/ttyS3					# Linux

# Get all .c files in src folder
SRC = $(wildcard ./src/*.c)
# All .c files in src map to .o files in build
OBJ = $(SRC:./src/%.c=./build/%.o)
DEP = $(OBJ:.o=.d)


# Make program
$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o ./build/$@.elf $(OBJ) $(LIB)
	avr-objcopy -j .text -j .data -O ihex ./build/$@.elf ./build/$@.hex

# .o files depend on .c files
./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

-include $(DEP)

./build/%.d: ./src/%.c | $(DIR)
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

# Create the build directory if it doesn't exist
$(DIR):
	mkdir $(DIR)


# Special commands
.PHONY: clean upload debug help #lib-common

# Remove all files in the build directory
clean:
	rm -f $(DIR)/*

# Upload program to board
upload: $(PROG)
	avrdude -c $(PGMR) -p $(MCU) -P $(PORT) -U flash:w:./build/$^.hex

# Print debug information
debug:
	@echo ————————————
	@echo $(SRC)
	@echo ————————————
	@echo $(OBJ)
	@echo ————————————

# Update and make lib-common
# lib-common:
# 	@echo "Fetching latest version of lib-common..."
# 	git submodule update --remote
# 	@echo "Compiling lib-common..."
# 	make -C lib-common clean
# 	make -C lib-common

# Help shows available commands
help:
	@echo "usage: make [clean | upload | debug | help]"
