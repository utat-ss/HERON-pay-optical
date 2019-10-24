#include "main.h"

volatile uint8_t spi_count = 0;
volatile uint8_t pin_state = 0;

int main(void) {
    init_board();

    print("Waiting for SPI\n");

    while(1){
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            if (spi_count != 0){
                print("SPI frames: %d Pin state %d\n", spi_count, pin_state);
                spi_count = 0;
            }
        }
    }
}


ISR(SPI_STC_vect){
    uint8_t ignore;
    if (!get_pin_val(PB2, &PORTB)){
        pin_state = 0;
    } else {
        pin_state = 1;
    }
    spi_count++;
    ignore = SPDR;
}

