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

uint8_t rx_command(const uint8_t* buf, uint8_t len);
void get_channel_readings(void);