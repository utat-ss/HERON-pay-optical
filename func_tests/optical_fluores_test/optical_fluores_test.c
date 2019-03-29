#include "../../src/optical_fluores.h"
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <spi/spi.h>

#include <test/test.h>

int main(void){
    uint8_t num = 1;
    init_opt_fluores();

    while(1){

        opt_fluores_on(num%8);
        _delay_ms(2000);

        opt_fluores_off(num%8);
        _delay_ms(2000);

        num++;

    }

    return 0;
}
