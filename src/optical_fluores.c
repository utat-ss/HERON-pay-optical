#include "optical_fluores.h"


// initializes blue fluorescent LED's
void init_opt_fluores(){
    print("\nInitializing blue fluorescent LEDs\n");;

    init_uart();
    print("\nInitialized uart");

    init_spi();
    print("\nInitialized spi");

    // disable channel 5, which is active low (see pay-optical schematic)
    init_output_pin(CH5_EN, &CH5_EN_DDR, 1);
    print("\nDisabled channel 5");

    init_pwm_16bit (0, 0xF7, 0x7B);
    print("\nInitialized pwm");

    syncdemod_init();
    syncdemod_enable_rclk(SD1_CS_PIN);
    syncdemod_enable_rclk(SD2_CS_PIN);
    syncdemod_enable_rclk(SD3_CS_PIN);
    syncdemod_enable_rclk(SD4_CS_PIN);
    print("\nInitialized syncdemod");
}


// sets fluores LED in channel num either on or off, by state
void opt_fluores_switch(uint8_t num, uint8_t state){
    switch (state){
        case (1):
            print("\nchannel %d: ", num);
            opt_adc_enable_mux(num);
            break;

        case (0):
            //syncdemod_disable_rclk(SD1_CS_PIN);
            break;
    }
}


// sets fluores LED in channel num on
void opt_fluores_on(uint8_t num){
    //opt_fluores_switch(num, 1);
    print("\nchannel %d: ", num);
    opt_adc_enable_mux(num);

    print("\nfluores on");
}


// sets fluores LED in channel num on
void opt_fluores_off(uint8_t num){
    opt_fluores_switch(num, 0);
    print("\nfluores off");
}
