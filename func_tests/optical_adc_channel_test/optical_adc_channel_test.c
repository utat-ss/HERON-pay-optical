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
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    opt_adc_init();
    print("Optical ADC initialized\n");

    print("\nStarting test\n\n");

    opt_adc_read_all_regs();

    while (1) {
        // Voltage is unipolar (pseudo-differential, pseudo bit = 1)
        // Voltage between AIN1-AIN16 to AINCOM
        for (uint8_t i = 1; i <= 16; i++) {
            uint32_t raw_data = opt_adc_read_channel_raw_data(i, 1);
            print("Channel #%u, Data = %06lX = %d %%\n", i, raw_data, (int16_t) ((double) raw_data / (double) 0xFFFFFF * 100.0));
            _delay_ms(2000);
        }

        print("\n");
        _delay_ms(10000);
    }
}
