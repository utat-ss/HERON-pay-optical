#include "main.h"

int main(void) {
	init_board();

    while (1) {
		all_off();
		_delay_ms(2000);
		print_power_info();
		all_on();
		_delay_ms(2000);
		print_power_info();
	}
}

void print_power_info(){
	print("-- Current draw %.3f\n", power_read_current());
	print("-- Sensor voltage %.3f\n", power_read_voltage());
	print("-- Sensor power %.3f\n\n", power_read_power());
}



