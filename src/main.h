#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <uart/uart.h>
#include "adc_optical.h"

#define DDR_SPI DDRB
#define DD_MISO DDB0

#define DATA_RDY_PIN    PD0
#define DATA_RDY_PORT   PORTD
#define DATA_RDY_DDR    DDRD

void spi_slave_init(void);
uint8_t spi_slave_receive(void);
void read_sensor_command(uint8_t field_number);

#endif
