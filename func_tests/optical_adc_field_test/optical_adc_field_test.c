#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
//#include <can/data_protocol.h>
#include <stdint.h>
#include <util/delay.h>
#include "../../src/optical_adc.h"
#include "../../src/pwm.h"

int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    opt_adc_init();
    print("Optical ADC initialized\n");

    init_pwm_16bit(0, 0xFF, 0x0F);
    print("16-bit PWM initialized\n");

    print("\nStarting test\n\n");

    while (1) {
        for (uint8_t i = 0; i < 36; i++) {
//        	uint32_t raw_data = opt_adc_read_field_raw_data(3);
            uint32_t raw_data = opt_adc_read_field_raw_data(i);
            print("Field #%u, Data = %06x = %d %%\n", i, raw_data, (int8_t) ((double) raw_data / (double) 0xFFFFFF * 100.0));
            _delay_ms(750);
        }

        print("\n\n\n\n");
        _delay_ms(2000);
    }
}
