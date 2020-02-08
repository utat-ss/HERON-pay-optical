#include "../../src/optical_spi.h"
#include "../../src/optical.h"
#include "../../src/power.h"
#include <util/delay.h>
#include <spi/spi.h>

// need some sort of global tracking variable
// bool spi_in_progress = 0;

int main(void) {
	// handles everything
	init_board();

	// Disable extra logging
	print_cal_info = false;

	// Change these to test each bank
	uint8_t start = 0;
	uint8_t end = 8;

	while (1) {
		print("\n");
		for (uint8_t field = start; field < end; field++) {
			// spi_second_byte contains well_info
			opt_update_reading(field);    // performs reading (3 bytes), stores it in wells[32] of well_t
			
			// fetch reading from registers

			uint32_t reading = 0;  
			if ( ((field >> OPT_TYPE_BIT) & 0x1) == PAY_OPTICAL)    // bit 5 = 1
				reading = (wells + (field & 0x1F))->last_opt_reading;
			else // PAY_LED, bit 5 = 1
				reading = (wells + (field & 0x1F))->last_led_reading;
			
			print("Field %u (0x%lx): ",
				field, reading);
			print("gain = 0x%lx, time = 0x%lx, data = 0x%lx\n",
				(reading >> 22) & 0x03, (reading >> 16) & 0x07, reading & 0xFFFF);
		}
	}

	return 0;
}
