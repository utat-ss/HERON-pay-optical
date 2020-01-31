#include "../../src/spi_comms.h"
#include "../../src/optical.h"
#include "../../src/power.h"
#include <util/delay.h>
#include <spi/spi.h>

// need some sort of global tracking variable
// bool spi_in_progress = 0;

int main(void) {
	// handles everything
	init_board();

	while (1) {
		// this value can be anything, interrupts handle all receiving/sending
		// _delay_ms(500);
        // print("hello world\n");

       opt_loop();
	}

	return 0;
}