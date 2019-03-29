#ifndef OPTICAL_SPI_H
#define OPTICAL_SPI_H

#include <stdbool.h>
#include <spi/spi.h>
#include "optical_adc.h"

// DATA_RDY pin - goes to PAY-SSM to tell it that optical data is ready
#define DATA_RDY_PIN    PD0
#define DATA_RDY_PORT   PORTD
#define DATA_RDY_DDR    DDRD

// Alternate MISO line
#define MISO_A_PIN      PD2
#define MISO_A_PORT     PORTD
#define MISO_A_DDR      DDRD

#define SPI_INTERRUPT_ENABLE    SPIE

extern bool opt_spi_use_dummy_data;

void opt_spi_init(void);

#endif
