#ifndef OPTICAL_SPI_H
#define OPTICAL_SPI_H

#include <stdbool.h>
#include <uart/uart.h>
#include "optical_adc.h"

#define DDR_SPI DDRB
#define DD_MISO DDB0

#define DATA_RDY_PIN    PD0
#define DATA_RDY_PORT   PORTD
#define DATA_RDY_DDR    DDRD

void opt_spi_init(void);
uint8_t opt_spi_receive(void);

#endif
