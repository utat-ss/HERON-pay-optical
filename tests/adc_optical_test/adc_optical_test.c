#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
#include <stdint.h>
#include <util/delay.h>
#include "../../src/adc_optical.h"

int main(void){
    init_uart();
    print("\n\n\nUART initialized\n");

    adc_optical_init();
    print("ADC Initialized\n");

    while (1) {
        for (uint8_t i = 0; i < 33; i++) {
            uint32_t raw_data = adc_optical_read_raw_data_field_number(i);
            print("Field %u, Data = %06x = %lf %%\n", i, raw_data, (double) raw_data / (double) 0xFFFFFF * 100.0);
        }

        print("\n\n");
        _delay_ms(10000);
    }
}
