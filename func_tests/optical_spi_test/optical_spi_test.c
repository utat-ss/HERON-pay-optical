#include "../../src/optical_spi.h"
#include <util/delay.h>
#include <uart/uart.h>

int main(void) {
	// DO NOT INITIALIZE UART - it interferes with the SPI slave functionality

	// Use dummy data instead of actual ADC data for testing
	opt_spi_use_dummy_data = true;

	opt_spi_init();

	while (1) {
		// this value can be anything, interrupts handle all receiving/sending
		_delay_ms(2500);
	}
}
