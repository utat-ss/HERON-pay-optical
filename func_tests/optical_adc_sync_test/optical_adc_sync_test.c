#include <uart/uart.h>
#include <can/data_protocol.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include <dac/dac.h>
#include <dac/pay.h>
#include "../../src/optical_adc.h"
#include "../../src/pwm.h"
#include "../../src/syncdemod.h"

#define CH5_EN_PORT     PORTC 
#define CH5_EN_DDR      DDRC    
#define CH5_EN          PIN2

/*
Yong Da Li
Monday, February 18, 2019

DAC_A and DAC_B is connected to AIN15 and AIN16, respectively
This test varies DAC_A and keeps DAC_B constant
Optical ADC uses AIN15(+ve CHN) and AIN16(-ve CHN) to measure differental voltage
from DAC_A and DAC_b

outputs raw data and percentage of full-scale value
full scale = +/- 2.5V --> 5.0V range
ex. output = 20%
    real value = 5.0V * 20% = 1.0V
    AIN15 and AIN16 differ by 1.0V
*/

// setup code for DAC
pin_info_t cs = {
    .pin = DAC_CS_PIN_PAY_OPT,
    .ddr = &DAC_CS_DDR_PAY_OPT,
    .port = &DAC_CS_PORT_PAY_OPT
};

pin_info_t clr = {
    .pin = DAC_CLR_PIN_PAY_OPT,
    .ddr = &DAC_CLR_DDR_PAY_OPT,
    .port = &DAC_CLR_PORT_PAY_OPT
};

dac_t dac = {
    .cs = &cs,
    .clr = &clr
};

int main(void){

    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI initialized\n");

    // Dylan's preamble magic to get stuff to work
    // ??
    init_cs(CH5_EN, &CH5_EN_DDR);
    set_cs_high(CH5_EN, &CH5_EN_PORT);

    opt_adc_init();
    print("Optical ADC initialized\n");

    init_pwm_16bit (0, 0xF7, 0x7B);
    print("16-bit PWN initialized\n");

    syncdemod_init();
    syncdemod_enable_rclk(SD4_CS_PIN);
    opt_adc_enable_mux(2);

    // Enable true differential output
    // Pseudo bit = 0
    // Enable bipolar mode = 0
    // read AIN 15 and 16 = VoutA and VoutB
    opt_adc_init_sync(4);
    print("initialized differential mode: AIN15+ve, AIN16-ve\n");

    // setup DAC
    // DAC_A and DAC_B are connected to AIN15 and AIN16 respectively
    // set DAC output to known voltage and read it using optical-adc
    // see "pay-optical" schematic, (p. 2 = "ADC Top Level")
    init_dac(&dac);
    print("DAC Initialized\n");
    print("\n");

    // VOUTB set to constant 1.5V 
    dac_set_voltage(&dac, 1.5, DAC_B);
    print("Set VOUTB = 1.5 V\n");

    print("\nStarting test\n\n");

    while (1) {
        // Voltage is bipolar, differential reading
        // +/- 2.5V, so 5.0V range

        // check config --> pseudo = 0 and bipolar input = 0
        // (p. 25)
        opt_adc_read_all_regs();

        for (uint8_t i = 0; i<10; i++){
            //VOUTA increases from 0 to 1.0V while VOUTB is constant at 1.5V
            dac_set_voltage(&dac, i*0.1, DAC_A);
            print("\nSet VOUTA = %d * 0.1 V\n", i);

            //display raw data
            uint32_t data = opt_adc_read_sync();
            print("Data = %06lX = %d %%\n", data, (int16_t) ((double) data / (double) 0xFFFFFF * 100.0));
            _delay_ms(500);
        }
    }
}