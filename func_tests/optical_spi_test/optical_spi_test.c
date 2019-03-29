#include "../../src/optical_spi.h"
#include <util/delay.h>
#include <spi/spi.h>

int main(void) {
	// DO NOT INITIALIZE UART - it interferes with the SPI slave functionality

	// Use dummy data instead of actual ADC data for testing
	opt_spi_use_dummy_data = false;

	opt_spi_init();

	while (1) {
		// this value can be anything, interrupts handle all receiving/sending
		_delay_ms(2500);
	}
}
