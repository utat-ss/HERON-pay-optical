#include <uart/uart.h>
#include <can/data_protocol.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include "../../src/optical_adc.h"
#include "../../src/pwm.h"
#include "../../src/syncdemod.h"

#define CH5_EN_PORT     PORTC 
#define CH5_EN_DDR      DDRC    
#define CH5_EN          PIN2

int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    init_cs(CH5_EN, &CH5_EN_DDR);
    set_cs_high(CH5_EN, &CH5_EN_PORT);

    opt_adc_init();
    print("Optical ADC initialized\n");

    init_pwm_16bit (0, 0xF7, 0x7B);
    print("16-bit PWN initialized\n");

    syncdemod_init();
    syncdemod_enable_rclk(SD4_CS_PIN);
    opt_adc_enable_mux(2);

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
