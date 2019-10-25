#ifndef __AVR_ATmega328__ 
#define __AVR_ATmega328__
#endif 

#ifndef SPI_COMMS_H
#define SPI_COMMS_H

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>
#include <stdint.h>

#define SS_PIN          PB2
#define SS_PORT         PORTB
#define SS_DDR          DDRB
#define SS_PINR         PINB


#endif // SPI_COMMS_H
