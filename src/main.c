#include <uart/uart.h>
#include "optical_adc.h"
#include "optical_spi.h"

int main(void) {
    // DO NOT INITIALIZE UART - it interferes with the SPI slave functionality

    // TODO - watchdog?

    // TODO - initialize optical ADC, SD, etc., use real data
    opt_spi_use_dummy_data = true;

    opt_spi_init();

    while (1) {}
}
