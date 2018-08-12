#include <uart/uart.h>
#include "optical_adc.h"
#include "optical_spi.h"

int main(void) {
    init_uart();
    print("UART Initialized\n");

    opt_adc_init();
    print("Optical ADC Initialized\n");

    opt_spi_init();
    print("SPI Slave Initialized\n");

    init_cs(DATA_RDY_PIN, &DATA_RDY_DDR);
    set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
    print("DATA_RDY Initialized\n");

    while (1) {}
}
