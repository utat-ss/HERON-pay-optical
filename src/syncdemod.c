/*
DESCRIPTION: ADA2200 functions
AUTHORS: Dylan Vogel

Contains functions for interfacing with the ADA2200 synrchonous demodulator on
the pay-optical PCB

TODO:
* Implement syncdemod_enable_rclk
* Implement functions to just set bandpass filter
* Consider implementing a reset toggle function, since reset can be held low for
  an indefinite amount of time.
*/

void syncdemod_init(void){
    uint8_t sd_cs_pin;
    // SD CS's are PC3 to PC6
    for (int i = 0; i < 4; i++){
        sd_cs_pin = i + 3;
        init_cs(sd_cs_pin, &SD_CS_DDR);
        set_cs_high(sd_cs_pin, &SD_CS_PORT);
        // Write the default configuration
        syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, SD_SERIAL_CONFIG_DEFAULT);
    }
}
void syncdemod_write_register(uint8_t sd_cs_pin, uint16_t addr, uint8_t data){
    uint16_t command;
    // Write is indicated by a 0 in bit 15. Read would be 1 in bit 15 (pg 19/24)
    command = 0x7FFF & addr;
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
    send_spi((uint8_t)(command >> 8 ));
    send_spi((uint8_t)(command));
    for (int i = 0; i < len; i++){
        send_spi(data[i]);
    }
}

void syncdemod_set_filters(uint8_t* config_arr){

}

void syncdemod_enable_rclk(void){

}
void syncdemod_disable_rlk(void){

}

void syncdemod_reset(uint8_t sd_cs_pin){
    // Resets the specified device registers

    uint16_t command;
    // Setting bit 7 and bit 0 to 1 resets the device
    command = SD_SERIAL_CONFIG_DEFAULT | (0b10000001);
    syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, command);
    _delay_ms(1);
    command &= (0b01111110);
    syncdemod_write_register(sd_cs_pin, SD_SERIAL_CONFIG_ADDR, command);
}
