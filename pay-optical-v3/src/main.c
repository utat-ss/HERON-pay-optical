#ifndef __AVR_ATmega8A__ 
#define __AVR_ATmega8A__
#endif 

#include <utilities/utilities.h>
#include <uart/uart.h>



int main(void) {
	init_uart();
    while (1) {
		_delay_ms(1000);
		print("Hello world\n");
    }
}

