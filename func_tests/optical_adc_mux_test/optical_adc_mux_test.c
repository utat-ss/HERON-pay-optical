/*
Test using the optical ADC's GPIO pins to control the multiplexers.
Note from the ADC the channels are numbered 0-7, but on the multiplexer component
they are named S1-S8.

Typical on resistance: 185 ohms (p.7)
*/

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
#include <can/data_protocol.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include "../../src/optical_adc.h"

// Need volatile because C does not interpret the interrupt
volatile bool key_pressed = false;

uint8_t echo(const uint8_t* buf, uint8_t len) {
    if (len == 0) {
        return 0;
    } else {
        key_pressed = true;
        return 1;
    }
}

void wait_key_press(void) {
    print("Waiting for key press...\n");
    while (!key_pressed) {}
    key_pressed = false;
}

int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    opt_adc_init();
    print("Optical ADC initialized\n");

    register_callback(echo);
    print("\nStarting test\n\n");

    opt_adc_disable_mux();
    print("Disabled mux\n");
    wait_key_press();

    opt_adc_enable_mux(2);
    print("Enabled mux ch.2 (S3)\n");
    wait_key_press();

    opt_adc_disable_mux();
    print("Disabled mux\n");
    wait_key_press();

    while (1) {
        for (uint8_t i = 0; i < 8; i++) {
            opt_adc_enable_mux(i);
            print("Enabled mux ch.%u (S%u)\n", i, i + 1);
            wait_key_press();
        }

        opt_adc_disable_mux();
        print("Disabled mux\n");
        wait_key_press();
    }
}
