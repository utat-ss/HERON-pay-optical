/*
NOTE: When uploading a program to PAY-Optical, you might need to hold down the
RST button on PAY-SSM. The PAY-SSM switch must be in RUN mode for the reset
button to work.
*/

#ifndef __AVR_ATmega328__ 
#define __AVR_ATmega328__
#endif 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include <uart/uart.h>
#include <spi/spi.h>
#include <utilities/utilities.h>

#include "power.h"
#include "spi_comms.h"

int main(void) {
    init_board();

    while(1){
        opt_loop();
    }
}
