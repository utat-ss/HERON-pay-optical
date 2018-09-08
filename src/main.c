#include <uart/uart.h>
#include "optical_adc.h"
#include "optical_spi.h"

int main(void) {
    opt_adc_init();
    print("Optical ADC Initialized\n");

    opt_spi_init();
    print("Optical SPI Initialized\n");

    while (1) {}
}
