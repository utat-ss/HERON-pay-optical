#ifndef __AVR_ATmega8A__ 
#define __AVR_ATmega8A__
#endif 

#include <avr/io.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <pex/pex.h>
#include <i2c/i2c.h>
#include "optical.h"
#include "power.h"


void print_power_info(void);