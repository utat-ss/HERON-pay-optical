/*
    DESCRIPTION: ADA2200 functions
    AUTHORS: Dylan Vogel

    Contains functions for interfacing with the ADA2200 synrchonous demodulator on
    the pay-optical PCB

    TODO:
    -   Implement functions to just set bandpass filter coefficients (i.e. wtf are they)
*/

#include "syncdemod.h"
#include "uart/uart.h"

// default callback
void sd_nop(void) { }
sd_fn_t sd_callbacks[4] = {sd_nop, sd_nop, sd_nop, sd_nop};

void syncdemod_init() {
    uint8_t sd_cs_pin;
    // SD CS's are PC3 to PC6
    for (int i = 0; i < 4; i++){
        sd_cs_pin = i + 3;
        // initialize the SPI pin behaviour
        init_cs(sd_cs_pin, &SD_CS_DDR);
        set_cs_high(sd_cs_pin, &SD_CS_PORT);
        // reset the synchronous demadulator
        syncdemod_reset(sd_cs_pin);
        // Write the default configuration register
        syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, SD_SERIAL_CONFIG_DEFAULT);
        // disable the RCLK output
        syncdemod_disable_rclk(sd_cs_pin);
        // write the default demod control, which controls RCLK pin behaviour
        syncdemod_write_register(sd_cs_pin, SD_DEMOD_CONTROL_ADDR, SD_DEMOD_CONTROL_DEFAULT);
        // enable the external clock input
        syncdemod_enable_external_clk(sd_cs_pin);
    }
}

void syncdemod_write_register(uint8_t sd_cs_pin, uint16_t addr, uint8_t data){
    // Write is indicated by a 0 in bit 15. Read would be 1 in bit 15 (pg 19/24)
    uint16_t command = 0x7FFF & addr;
    // Four different SDs they could write to.
    // User should specify SD1, SD2, SD3 or SD4
    set_cs_low(sd_cs_pin, &SD_CS_PORT);

    send_spi((uint8_t)(command >> 8));
    send_spi((uint8_t)(command));
    send_spi(data);

    set_cs_high(sd_cs_pin, &SD_CS_PORT);
}

void syncdemod_write_multibyte(uint8_t sd_cs_pin, uint16_t addr, uint8_t len, uint8_t* data){
    uint16_t command;

    command = 0x7FFF & addr;

    set_cs_low(sd_cs_pin, &SD_CS_PORT);

    send_spi((uint8_t)(command >> 8));
    send_spi((uint8_t)(command));
    for (int i = 0; i < len; i++){
        send_spi(data[i]);
    }

    set_cs_high(sd_cs_pin, &SD_CS_PORT);
}

void syncdemod_set_filters(uint8_t sd_cs_pin, uint8_t* config_arr){
    // Write 0x03 to filter strobe register to load the coefficients
    uint16_t command = 0x03;
    // Write the filter coefficients
    syncdemod_write_multibyte(sd_cs_pin, SD_FILTER_CONFIG_BASE_ADDR, 23, config_arr);
    // Get the filters set
    syncdemod_write_register(sd_cs_pin, SD_FILTER_STROBE_ADDR, command);
    // write 0x00 to reset the strobe register (might not be needed?)
    command = 0x00;
    syncdemod_write_register(sd_cs_pin, SD_FILTER_STROBE_ADDR, command);
}

void syncdemod_set_clock(uint8_t sd_cs_pin, int clkin_div, uint8_t rclk_div){
    uint8_t clkin_bits, rclk_bits, command;

    switch(clkin_div){
        // default (on reset) is clkin div == 1
        case 1:
            clkin_bits = 0x00;
            break;
        case 16:
            clkin_bits = 0x01;
            break;
        case 64:
            clkin_bits = 0x02;
            break;
        case 256:
            clkin_bits = 0x04;
            break;
        default:
            clkin_bits = 0x00;
    }

    switch (rclk_div){
        // default (on reset) is rclk div == 8
        case 4:
            rclk_bits = 0x01;
            break;
        case 8:
            rclk_bits = 0x02;
            break;
        default:
            rclk_bits = 0x02;
    }

    command = (clkin_bits << 2) | rclk_bits;
    print ("clock command: %u\n", command);
    syncdemod_write_register(sd_cs_pin, SD_CLK_CONFIG_ADDR, command);
}

void syncdemod_write_synco(uint8_t sd_cs_pin, uint8_t enable, uint8_t polarity, uint8_t edge){
    // bit 5 is synco enable (active high)
    // bit 4 is polarity (default == 0)
    // bit[3:0] is edge selection for the synco output (default == 1101)
    uint16_t command = ((0x01 & enable) << 5) | ((0x01 & polarity) << 4) | (0x0F & edge);
    syncdemod_write_register(sd_cs_pin, SD_SYNC_CONTROL_ADDR, command);
}

void syncdemod_enable_external_clk(uint8_t sd_cs_pin){
    // enables CMOS level clock on the CLKIN pin
    uint16_t command = 0x01;
    syncdemod_write_register(sd_cs_pin, SD_ANALOG_CONFIG_ADDR, command);
}

void syncdemod_disable_external_clk(uint8_t sd_cs_pin){
    // disables the CMOS level clock and enables the external crystal
    uint16_t command = 0x00;
    syncdemod_write_register(sd_cs_pin, SD_ANALOG_CONFIG_ADDR, command);
}

void syncdemod_enable_rclk(uint8_t sd_cs_pin){
    // set bit 0 to 1 to enable RCLK/SDO pin
    uint16_t command = 0x01;
    syncdemod_write_register(sd_cs_pin, SD_DIGITAL_CONFIG_ADDR, command);
}

void syncdemod_disable_rclk(uint8_t sd_cs_pin){
    // Set bit 0 to 0 to disable RCLK/SDO pin
    uint16_t command = 0x00;
    syncdemod_write_register(sd_cs_pin, SD_DIGITAL_CONFIG_ADDR, command);
}

void syncdemod_reset(uint8_t sd_cs_pin){
    // Resets the specified device
    uint16_t command;
    // Setting bit 7 and bit 0 to 1 resets the device
    command = SD_SERIAL_CONFIG_DEFAULT | (0x80);
    syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, command);
    _delay_ms(1);
    command = SD_SERIAL_CONFIG_DEFAULT;
    syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, command);
}

void syncdemod_toggle_core_reset(uint8_t sd_cs_pin){
    // if reset_state == 0, device is not reset, and vice-versa
    static uint8_t reset_state = 0x00;
    switch (reset_state){
        case 0x00:
            reset_state = 0x01;
        case 0x01:
            reset_state = 0x00;
    }
    syncdemod_write_register(sd_cs_pin, SD_CORE_RESET_ADDR, reset_state);
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> Wrote initialization for interrupts and test file.
}

/*
Initialize interrupt pins INT0-INT3 and set callback functions
*/
void syncdemod_init_interrupt(sd_fn_t f0, sd_fn_t f1, sd_fn_t f2, sd_fn_t f3) {
    // set behavior of all interrupts to trigger on rising edge, pg 71
    EICRA = 0xFF;
    // enable external interrupt 0-3
    EIMSK |= 0x0F;
    // enable global interrupts
    sei();

    // set callback functions
    sd_callbacks[0] = f0;
    sd_callbacks[1] = f1;
    sd_callbacks[2] = f2;
    sd_callbacks[3] = f3;
}

/*
*/
ISR(INT0_vect) {
    sd_callbacks[0]();
}


ISR(INT1_vect) {
    sd_callbacks[1]();
}


ISR(INT2_vect) {
    sd_callbacks[2]();
}


ISR(INT3_vect) {
    sd_callbacks[3]();
}
<<<<<<< HEAD
=======
}
>>>>>>> DOESN'T WORK: adc sync init and read
=======
>>>>>>> Wrote initialization for interrupts and test file.
