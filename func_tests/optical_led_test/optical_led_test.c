/*
DESCRIPTION: toggles LEDs on pay-optical
AUTHOR: Yong Da Li
*/

#include "../../src/optical_led.h"
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <spi/spi.h>

#ifndef F_CPU
#define F_CPU 8000000UL //8 MHz clock
#endif

int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");
    print("\n");

    print("Starting test\n");

    init_opt_led();

    while(1){
        // toggle all LED's on/off from left-to-right, top-down
        // *actualy 34 LEDs and 2 empty spots
        for (uint8_t i = 0; i<36; i++){
            opt_led_board_position_on(i);
            _delay_ms(250);
            opt_led_board_position_off(i);
        }
    }
   
}
