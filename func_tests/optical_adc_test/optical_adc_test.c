#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
#include <can/data_protocol.h>
#include <stdint.h>
#include <util/delay.h>
#include "../../src/optical_adc.h"

int main(void){
    init_uart();
    print("\n\n\nUART initialized\n");

    opt_adc_init();
    print("Optical ADC Initialized\n");

    while (1) {
        for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; i++) {
            uint32_t raw_data = opt_adc_read_field_raw_data(i);
            print("Field #%u, Data = %06x = %d %%\n", i, raw_data, (int8_t) ((double) raw_data / (double) 0xFFFFFF * 100.0));
        }

        print("\n");
        _delay_ms(10000);
    }
}
