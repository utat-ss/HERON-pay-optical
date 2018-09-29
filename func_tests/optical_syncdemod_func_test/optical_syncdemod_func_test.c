#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
//#include <can/data_protocol.h>
#include <stdint.h>
#include"../../src/syncdemod.h"
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

    // 160KHz, 50% duty input signal to the syncdemod
    // (8MHz / 50 = 160KHz)
    // should result in an RCLK of 2.5KHz
    init_pwm_16bit(0, 0x32, 0x16);
    print("16-bit PWM initialized\n");

    print("\nStarting test\n\n");

    opt_adc_enable_mux(2);
    syncdemod_enable_rclk(SD4_CS_PIN);
    while (1) {
    }
}
