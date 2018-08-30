#ifndef OPTICAL_SPI_H
#define OPTICAL_SPI_H

#include <stdbool.h>
#include <uart/uart.h>
#include "optical_adc.h"

#define DATA_RDY_PIN    		PD0
#define DATA_RDY_PORT   		PORTD
#define PORT_D_DDR	    		DDRD
#define DATA_RDY_DD				DDD0
#define MISO_A_DD				DDD2
#define SPI_INTERRUPT_ENABLE	SPIE

void opt_spi_init(void);

#endif
