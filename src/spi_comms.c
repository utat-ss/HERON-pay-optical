#include "spi_comms.h"

volatile uint8_t spi_spdr_buffer;

/*
Initialize SPI comms
*/
void init_spi_comms(void){
    spi_spdr_buffer = 0;
    init_spi();
}

ISR(SPI_STC_vect){
    // check if SS is still low
    if (!(SS_PINR & _BV(SS_PIN))){
        while (!(SPSR & _BV(SPIF)));
    }
}

