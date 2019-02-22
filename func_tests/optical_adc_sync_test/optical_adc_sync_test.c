#include <uart/uart.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include <conversions/conversions.h>
#include <dac/dac.h>
#include <dac/pay.h>
#include "../../src/optical_adc.h"

#define CH5_EN_PORT     PORTC
#define CH5_EN_DDR      DDRC
#define CH5_EN          PIN2

/*
Yong Da Li
Monday, February 18, 2019

Optical ADC uses AIN15(+ve CHN) and AIN16(-ve CHN) to measure differental voltage
DAC_A and DAC_B is connected to AIN15 and AIN16, respectively
This test varies both DAC_A and DAC_B to test the full differential range of the ADC

Prints raw data, percentage of full-scale value and converted voltage
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

    init_cs(CH5_EN, &CH5_EN_DDR);
    set_cs_high(CH5_EN, &CH5_EN_PORT);

    opt_adc_init();
    print("Optical ADC initialized\n");

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

    print("\nStarting test\n\n");

    while (1) {
        // Voltage is bipolar, differential reading
        // +/- 2.5V, so 5.0V range

        // check config --> pseudo = 0 and bipolar input = 0
        // (p. 25)
        //opt_adc_read_all_regs();

        for (uint8_t i = 0; i<=25; i++){
            //VOUTA increases from 0 to 1.0V while VOUTB is constant at 1.5V
            set_dac_voltage(&dac, DAC_A, i*0.1);
            print("\nSet VOUTA = %.2f V", i*0.1);
            set_dac_voltage(&dac, DAC_B, (25-i)*0.1);
            print("\nSet VOUTB = %.2f V\n", (25-i)*0.1);

            //display raw data
            uint32_t data = opt_adc_read_sync();
            print("Data = %06lX = %d %%\n", data, (int16_t) ((double) data / (double) 0xFFFFFF * 100.0));
            print("Voltage = %.2f V \n\n", opt_adc_raw_data_to_diff_vol(data, 1));
            _delay_ms(500);
        }
    }
}
