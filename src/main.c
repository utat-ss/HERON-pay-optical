#include <uart/uart.h>

#define DDR_SPI DDRB
#define DD_MISO DDB0

// Interrupts - see datasheet section 8
// SPI - see datasheet pg.133-...

// TODO - is this necessary? just interrupt?
void spi_slave_init(void) {
    /* Set MISO output, all others input */
    DDR_SPI = (1<<DD_MISO);
    /* Enable SPI */
    SPCR = (1<<SPE);
}

uint8_t spi_slave_receive(void) {
    /* Wait for reception complete */
    while(!(SPSR & (1<<SPIF)));
    /* Return data register */
    return SPDR;
}

// TODO - after a SPI transfer finishes
ISR(SPI_STC_vect) {
    print("SPI serial transfer complete\n");
}

// TODO - place SPI data in SPDR beforehand
// maybe 1 for data ready, 0 for not ready?
int main(void) {
    init_uart();
    print("Hello World!\n");
}
