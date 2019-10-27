# Reference: http://www.atmel.com/webdoc/avrlibcreferencemanual/group__demo__project_1demo_project_compile.html

# Don't change these
# AVR-GCC compiler
CC = avr-gcc
# Compiler flags
CFLAGS = -Wall -std=gnu99 -Wl,-u,vfprintf -g -mmcu=atmega328 -Os -mcall-prologues
# Includes (header files)
INCLUDES = -I../../lib-common-ported/include/
# Programmer
PGMR = stk500
# Microcontroller
MCU = m328

# Libraries from lib-common to link
# May need to change this line
LIB = -L../../lib-common-ported/lib -lpex -luart -lspi -li2c -lutilities -lprintf_flt -lm
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

# SRC - defined in the example-specific makefile
# All .c files in src map to .o files
OBJ = $(SRC:../../src/%.c=./%.o)

# Make program
$(PROG): $(PROG).o $(OBJ)
	$(CC) $(CFLAGS) -o $@.elf $^ $(LIB)
	avr-objcopy -j .text -j .data -O ihex $@.elf $@.hex

# .o files depend on .c files
$(PROG).o: $(PROG).c
	$(CC) $(CFLAGS) -c $(PROG).c $(INCLUDES)

./%.o: ../../src/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)


# Special commands
.PHONY: clean upload debug lib-common help

clean:
	rm -f ./*.o
	rm -f ./*.elf
	rm -f ./*.hex

upload: $(PROG)
	avrdude -p $(MCU) -c $(PGMR) -P $(PORT) -U flash:w:./$^.hex

# Print debug information
debug:
	@echo ————————————
	@echo $(SRC)
	@echo ————————————
	@echo $(OBJ)
	@echo ————————————

# Update and make lib-common
lib-common:
	@echo "Fetching latest version of lib-common..."
	git submodule update --remote
	@echo "Compiling lib-common..."
	make -C ../../lib-common clean
	make -C ../../lib-common

help:
	@echo "usage: make [clean | upload | debug | lib-common | help]"
