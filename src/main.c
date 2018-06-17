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

void setup_adc(void) {
    // TODO
    // init_port_expander();
    // init_adc();
    // print("ADC Setup Completed\n");
    //
    // adc_optical_write_register(CONFIG_ADDR, CONFIG_DEFAULT);
    // uint32_t config_data = read_ADC_register(CONFIG_ADDR);
    // print("ADC Configuration Register: %lX\n", config_data);
    //
    // int pga_gain = 1;
    // set_PGA(pga_gain);
    // print("PGA gain: %d\n", pga_gain);
}

// TODO - place SPI data in SPDR beforehand
// maybe 1 for data ready, 0 for not ready?
int main(void) {
    init_uart();
    print("Hello World!\n");
}
