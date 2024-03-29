#include <avr/io.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <pex/pex.h>
#include <i2c/i2c.h>
#include "../../src/optical.h"
#include "../../src/power.h"

void sequence(void){
    while(1){
		for (uint8_t i = 0; i < 32; i++){
			set_led(i, PAY_OPTICAL, 1);
            set_led(i, PAY_LED, 1);
			_delay_ms(500);
			set_led(i, PAY_OPTICAL, 0);
            set_led(i, PAY_LED, 0);
        }
    }
}

void sequence_2(void){
    while(1){
		for (uint8_t i = 0; i < 32; i++){
			set_led(i, PAY_OPTICAL, 1);
            _delay_ms(200);
            set_led(i, PAY_OPTICAL, 0);
            _delay_ms(100);
            set_led(i, PAY_LED, 1);
			_delay_ms(200);
            set_led(i, PAY_LED, 0);
            _delay_ms(100);
        }
    }
}

void sequence_3(void){
    while(1){
		for (uint8_t i = 16; i < 24; i++){
			set_led(i, PAY_OPTICAL, 1);
            _delay_ms(200);
            set_led(i, PAY_OPTICAL, 0);
            _delay_ms(100);
            set_led(i, PAY_LED, 1);
			_delay_ms(200);
            set_led(i, PAY_LED, 0);
            _delay_ms(100);
        }
    }
}

void all(void){
    while(1){
        all_on();
        _delay_ms(1000);
        all_off();
        _delay_ms(1000);
    }
}

int main(void) {
    init_board();
    all();
    // sequence();
    // sequence_2();
    // sequence_3();
}
