#include "main.h"

// TODO - watchdog?

int main(void) {
	uint32_t data = 0;
	uint8_t gain = 0;
	uint8_t time = 0;

	init_board();

	for (uint8_t i = 0; i < 32; i++){
		data = get_opt_sensor_reading(i, PAY_OPTICAL);
		gain = (uint8_t)(data >> 24);
		time = (uint8_t)((data >> 16) & 0x00FF);
		print("-- ,Sensor: %2d, Gain: %02X, Time: %02X, Value: %lu,\n", i, gain, time, (data & 0x0000FFFF));
	}
}



