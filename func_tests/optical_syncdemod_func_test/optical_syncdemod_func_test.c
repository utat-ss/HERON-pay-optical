#include <uart/uart.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include "../../src/optical_adc.h"
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"

#define CH5_EN_PORT		PORTC
#define CH5_EN_DDR		DDRC
#define CH5_EN			 PIN2

/*
Dylan Vogel

This test is meant to be used in conjunction with an oscilloscope. It will
enable syncchronous demodulator channels 1-4 and output the data.

If the SMAs are soldered onto channel 5, you can set the initial value of CH5_EN
to "0" instead of "1" to enable it.
*/


int main(void){

    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    init_output_pin(CH5_EN, &CH5_EN_DDR, 1);
    print("Disabled Channel 5\n");

    // Should give 32.258 KHz, divided down by 64 on the syncdemod
    // About 504.032 Hz at the output, 50% duty
    init_pwm_16bit (0, 0xF7, 0x7B);
    print("16-bit PWM initialized\n");

    opt_adc_init();
    print("Optical ADC initialized\n");

    syncdemod_init();
    syncdemod_enable_rclk(SD1_CS_PIN);
    syncdemod_enable_rclk(SD2_CS_PIN);
    syncdemod_enable_rclk(SD3_CS_PIN);
    syncdemod_enable_rclk(SD4_CS_PIN);
    print("Enabled all RCLK");

    // When the SMAs were soldered, this would correspond to the 1x
    // testing port.
    opt_adc_enable_mux(2);
    print("Selected multiplexer channel 2");

    print("\nStarting test\n\n");

    while (1) {
    }
}
