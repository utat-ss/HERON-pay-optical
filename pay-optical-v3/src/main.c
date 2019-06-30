#ifndef __AVR_ATmega8A__ 
#define __AVR_ATmega8A__
#endif 

#include <utilities/utilities.h>
#include <uart/uart.h>
#include "optical.h"



int main(void) {
	init_uart();
	print("-- UART Initialized\n");
	init_opt_pex();
	print("-- Port Expanders Initialized\n");
	print("-- Flashing LEDs\n");

    while (1) {
		all_on();
		_delay_ms(200);
		all_off();
		_delay_ms(200);
    }
}

