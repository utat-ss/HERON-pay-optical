#include "../../src/optical_spi.c"
#include <util/delay.h>
#include <uart/uart.h>

int main(void) {
	opt_spi_init();

	while (1) {
		// this value can be anything, interrupts handle all receiving/sending
		_delay_ms(2500);
	}
}
