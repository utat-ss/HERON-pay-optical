/*
DESCRIPTION: AD7194 ADC functions
AUTHORS: Bruno Almeida, Dylan Vogel, Yong Da Li

Datasheet: http://www.analog.com/media/en/technical-documentation/data-sheets/AD7194.pdf
FAQ: http://www.analog.com/media/en/technical-documentation/frequently-asked-questions/AD719x_FAQ_Instru_Conv.pdf

This is referred to as the "optical ADC", which is different from the primary
ADC library in lib-common.

- single conversion mode

GPIO pins:
P3 - _EN_
P2 - A2
P1 - A1
P0 - A0

TODO:
* Add a function to automatically scale gain
* Add calibration function
* Use 4 GPIO pins for mux A0, A1, A2, EN
* Switch GPIO outputs to mux when appropriate, create function

* create raw_data to actual voltage conversion function
* decrease settling time of ADC (to get faster read speed, current ~33ms)
*       - consider various filtering options on ADC
*/

#include "optical_adc.h"
#include "syncdemod.h"


void opt_adc_init(void){
    // Initialize ports and registers needed for ADC usage

    // Set _CS_ high
    init_cs(CS_PIN, &CS_DDR);
    set_cs_high(CS_PIN, &CS_PORT);

    // Set _SYNC_ high
    init_cs(SYNC_PIN, &SYNC_DDR);
    set_cs_high(SYNC_PIN, &SYNC_PORT);

    opt_adc_reset();

    // opt_adc_write_reg(CONFIG_ADDR, CONFIG_DEFAULT);
    // "Continuous conversion is the default power-up mode." (p. 32)

    // GPOCON register - enable 4 GPIO outputs
    // _EN_ = 1, A2/A1/A0 = 0
    opt_adc_write_reg(GPOCON_ADDR, GPOCON_SETTING);

    opt_adc_init_config();
    // opt_adc_init_mode();
    opt_adc_select_op_mode(MODE_POWER_DOWN);
    syncdemod_init();
}


void opt_adc_reset(void) {
    // p.32 - "If a Logic 1 is written to the AD7194 DIN line for at least 40 serial clock cycles, the serial interface is reset."
    // Just use 48 clock cycles
    // TODO - does CS need to be low?
    set_cs_low(CS_PIN, &CS_PORT);
    for (uint8_t i = 0; i < 6; i++) {
        send_spi(0xFF);
    }
    set_cs_high(CS_PIN, &CS_PORT);

    // p.32 - "Following a reset, the user should allow a period of 200 μs before addressing the serial interface."
    _delay_us(200);
}


// Initializes configuration register
void opt_adc_init_config(void) {
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);

    // Enable pseudo-differential output
    // Enable unipolar (positive voltage) mode
    // Set gain to 0b000 (PGA = 1), by CONFIG_MASK
    // Pseudo bit = 1
    config = config & CONFIG_MASK;
    config = config | CONFIG_PSEUDO_ON;
    config = config | CONFIG_UNIPOLAR;

    //set gain to 1
    opt_adc_select_pga(1);

    opt_adc_write_reg(CONFIG_ADDR, config);
}


// Initializes continous sync mode
// pair_num 0 = AIN5+ve, AIN6-ve
// pair_num 1 = AIN7+ve, AIN8-ve
// pair_num 2 = AIN9+ve, AIN10-ve
// pair_num 3 = AIN11+ve, AIN12-ve
// pair_num 4 = AIN15+ve, AIN16-ve
void opt_adc_init_sync(uint8_t pair_num){
    uint32_t channel_pos = 0, channel_neg = 0;

    opt_adc_select_differential();

    switch(pair_num){
        case 0:
            channel_pos = 5;
            channel_neg = 6;
            break;
        case 1:
            channel_pos = 7;
            channel_neg = 8;
            break;
        case 2:
            channel_pos = 9;
            channel_neg = 10;
            break;
        case 3:
            channel_pos = 11;
            channel_neg = 12;
            break;
        case 4:
            channel_pos = 15;
            channel_neg = 16;
    }

    opt_adc_select_channel(channel_pos, 1);
    opt_adc_select_channel(channel_neg, 0);

    opt_adc_select_op_mode(MODE_CONT_CONV);

    _delay_ms(75);  // equivalent to about 60,000 timeout cycles

    //wait 4 master clock cycles (1us) for synchronization
    // (p. 36)
    set_cs_low(SYNC_PIN, &SYNC_PORT);

    //probably don't need the 1us delay, since 32m1 takes
    //some time to read instructions anyway
    //_delay_us(1);
}


uint32_t opt_adc_read_reg(uint8_t register_addr) {
    // Read the current state of the specified ADC register.
    register_addr = register_addr & REGISTER_ADDRESS_MASK;

    // Start communication
    set_cs_low(CS_PIN, &CS_PORT);

    // Begin communication with a write to the communications register
    // Request a read from the specified register
    uint8_t spi_tx = COMM_READ | (register_addr << 3);
    send_spi(spi_tx);
    // print("Read register request: %02X\n", spi_tx);

    // Read the required number of bytes based on register
    uint32_t data = 0;
    uint8_t num_bytes = opt_adc_num_reg_bytes(register_addr);
    // print("%u bytes\n", num_bytes);
    for (uint8_t i = 0; i < num_bytes; i++) {
        uint8_t next_byte = send_spi(0x00);
        // print("Received byte: %02X\n", next_byte);
        data = (data << 8) | next_byte;
    }

    // Stop communication
    set_cs_high(CS_PIN, &CS_PORT);

    // print("Read register data: %06lX\n", data);

    return data;
}


void opt_adc_write_reg(uint8_t register_addr, uint32_t data) {
    // Writes a new state to the specified ADC register.
    register_addr = register_addr & REGISTER_ADDRESS_MASK;

    // Start communication
    set_cs_low(CS_PIN, &CS_PORT);

    // Begin communication with a write to the communications register
    // Request a write to the specified register
    uint8_t spi_tx = COMM_WRITE | (register_addr << 3);
    send_spi(spi_tx);
    // print("Write register request: %02X\n", spi_tx);

    // print("Write register data: %06lX\n", data);

    // Write the number of bytes in the register
    uint8_t num_bytes = opt_adc_num_reg_bytes(register_addr);
    // print("%u bytes\n", num_bytes);
    for (int8_t i = num_bytes - 1; i >= 0; i--) {
        uint8_t next_byte = data >> (i * 8);
        send_spi(next_byte);
        // print("Sent byte: %02X\n", next_byte);
    }

    // Set CS high
    set_cs_high(CS_PIN, &CS_PORT);
}


void opt_adc_select_channel(uint8_t channel_num, uint8_t sign) {
    // Sets the configuration register's bits for the specified ADC input channel.
    // channel_num must be between 1 and 16
    // accepts 1, 0 for +/- sign, uni-polar is always +, differential requires + and - (p. 26)

    // Read the current register status
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);

    // Get the 4 bits for the channel for psuedo-differential positive input (p. 26)
    if (sign){ // +ve
        uint8_t channel_bits = (channel_num - 1) << 4;
        // Mask the channel bits and write new channel
        config &= CHANNEL_MASK_POS;
        config |= ((uint32_t) channel_bits) << 8;
        print("AIN %d set to positive\n", channel_num);
    }
    else{ // -ve
        uint8_t channel_bits = (channel_num -1);
        config &= CHANNEL_MASK_NEG;
        config |= ((uint32_t) channel_bits) << 8;
        print("AIN %d set to negative\n", channel_num);
    }

    // Write the new config register value
    opt_adc_write_reg(CONFIG_ADDR, config);
}


void opt_adc_select_pga(uint8_t gain) {
    // Sets the configuration register's bits for a specified programmable gain.
    // gain - one of 1, 8, 16, 32, 64, 128 (2 and 4 are reserved/unavailable, see p. 25)

    // Convert gain to 3 bits
    uint8_t gain_bits = opt_adc_gain_to_gain_bits(gain);

    // Read from configuration register
    uint32_t config_data = opt_adc_read_reg(CONFIG_ADDR);

    // Clear gain bits and set
//    config_data &= 0xfffff8;
    config_data &= PGA_MASK;
    config_data |= gain_bits;

    // Write to configuration register
    opt_adc_write_reg(CONFIG_ADDR, config_data);
}


// Selects the operating mode
void opt_adc_select_op_mode(uint8_t mode_bits) {
    // Read from mode register
    uint32_t mode_reg = opt_adc_read_reg(MODE_ADDR);

    // Clear mode bits and set
    mode_reg &= MODE_MASK;
    mode_reg |= ((uint32_t) mode_bits) << 21;

    // Write to configuration register
    opt_adc_write_reg(MODE_ADDR, mode_reg);
}


// sets pseudo bit to 0 for differential input
// turns on bipolar input mode --> bit = 0
void opt_adc_select_differential(void){
    // Read from mode register
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);

    // Enable differential output
    // Enable bipolar mode
    // Pseudo bit = 0
    config = config & CONFIG_MASK;
    config = config & CONFIG_PSEUDO_OFF;
    config = config & CONFIG_BIPOLAR;

    opt_adc_write_reg(CONFIG_ADDR, config);
}


// See the single conversion mode, p.33
uint32_t opt_adc_read_channel_raw_data(uint8_t channel_num, uint8_t gain) {
    // Reads 24 bit raw data from the specified ADC channel.

    // Select the channel for conversion
    opt_adc_select_channel(channel_num, 1);
    // TODO - store gain variable and check if it changed?
    // TODO - set gain
    // opt_adc_select_pga(gain);
    opt_adc_select_op_mode(MODE_SINGLE_CONV);

    /*
    p.33 - "DOUT/RDY goes low to indicate the completion of a conversion."

    FAQ p.10
    "the user can take CS low, initiate the single conversion and then take CS high again...
    When CS is taken high, the DOUT/RDY pin is tristated. Therefore, the DOUT/RDY pin will not indicate the end of the conversion."

    Wait until the conversion finishes, signalled by (DOUT/_RDY_/MISO) going low
    Must set _CS_ low first for this to work

    In testing, 200us is about 150 timeout cycles (about 1.333us per timeout cycle)
    In testing, the ADC reading/conversion takes about 63,150 timeout cycles (if no delays), i.e. about 84ms
    */

    _delay_ms(75);  // equivalent to about 60,000 timeout cycles
    set_cs_low(CS_PIN, &CS_PORT);
    uint16_t timeout = 65535;
    while (bit_is_set(PINB, MISO_PIN) && timeout > 0) {
        timeout--;
    }
    // print("Waited for %u cycles\n", 65535 - timeout);
    if (timeout == 0) {
        print("ERROR: TIMEOUT\n");
    } else {
        print("Conversion successful\n");
    }

    // Read back the conversion result
    return opt_adc_read_reg(DATA_ADDR);
}


uint8_t opt_adc_num_reg_bytes(uint8_t register_addr) {
    // Returns the number of BYTES in the specified register at the given address

    switch (register_addr) {
        case STATUS_ADDR:
            return 1;
            break;
        case MODE_ADDR:
            return 3;
            break;
        case CONFIG_ADDR:
            return 3;
            break;
        case DATA_ADDR:
            return 3;
            // TODO - check byte of status information (enabled by setting DAT_STA, p. 20-21)
            break;
        case ID_ADDR:
            return 1;
            break;
        case GPOCON_ADDR:
            return 1;
            break;
        case OFFSET_ADDR:
            return 3;
            break;
        case FULL_SCALE_ADDR:
            return 3;
            break;
        default:
            return 0;
            break;
    }
}


uint8_t opt_adc_gain_to_gain_bits(uint8_t gain) {
    // Converts the numerical gain to 3 gain select bits (p.25).
    // gain - one of { 1, 8, 16, 32, 64, 128 }

    switch (gain) {
        case 1:
            return 0b000;
            break;
        case 8:
            return 0b011;
            break;
        case 16:
            return 0b100;
            break;
        case 32:
            return 0b101;
            break;
        case 64:
            return 0b110;
            break;
        case 128:
            return 0b111;
            break;
        default:
            return 0b000;
            break;
    }
}


// channel - channel to select (between 0 and 7 to represent S1-S8)
void opt_adc_enable_mux(uint8_t channel) {
    uint8_t channel_bits = channel;

    // Set _EN_ (bit 3) = 0, bits 2-0 = channel
    uint8_t gpocon = opt_adc_read_reg(GPOCON_ADDR);
    gpocon = gpocon & GPOCON_MASK;
    gpocon = gpocon | (0x3 << 4);
    gpocon = gpocon | channel_bits;
    opt_adc_write_reg(GPOCON_ADDR, gpocon);
}


void opt_adc_disable_mux(void) {
    // Set _EN_ (bit 3) = 1
    uint8_t gpocon = opt_adc_read_reg(GPOCON_ADDR);
    gpocon = gpocon | _BV(3);
    opt_adc_write_reg(GPOCON_ADDR, gpocon);
}


// Reads 24 bits of raw data from the specified field, using the system of 5
// multiplexors with 8 pins each
uint32_t opt_adc_read_field_raw_data(uint8_t field_number) {
    uint8_t group = field_number / 8;
    uint8_t address = field_number % 8;

    // determine which ADC channel and syncdemod CS pin to use
    uint8_t adc_channel;
    uint8_t sd_cs_pin;
    switch (group) {
        case 0:
            adc_channel = 5;
            sd_cs_pin = SD1_CS_PIN;
            break;
        case 1:
            adc_channel = 7;
            sd_cs_pin = SD2_CS_PIN;
            break;
        case 2:
            adc_channel = 9;
            sd_cs_pin = SD3_CS_PIN;
            break;
        case 3:
            adc_channel = 11;
            sd_cs_pin = SD4_CS_PIN;
            break;
        case 4:
            adc_channel = 13;
            sd_cs_pin = SD4_CS_PIN;
            break;
        default:
            print("Unexpected sensor group\n");
            adc_channel = 5;
            sd_cs_pin = SD1_CS_PIN;
            break;
    }

    // Enable the mux for the appropriate address
    // (this should turn on the LED and enable the amplifier)
    opt_adc_enable_mux(address);
    syncdemod_enable_rclk(sd_cs_pin);
    // TODO: why is this delay here?
    _delay_ms(100);

    // Read ADC data and prepare to send it over SPI
    uint32_t raw_data = opt_adc_read_channel_raw_data(adc_channel, 1);
    syncdemod_disable_rclk(sd_cs_pin);
    opt_adc_disable_mux();

    return raw_data;
}


// reads continuous conversion mode, but without synchronization
// make sure operation mode is set to MODE_CONT_CONV before calling
uint32_t opt_adc_read_cont_conv(void){
    //begin communication
    set_cs_low(CS_PIN, &CS_PORT);

    uint16_t timeout = 65535;
    while (bit_is_set(PINB, MISO_PIN) && timeout > 0) {
        timeout--;
    }

    print("Waited for %u cycles\n", 65535 - timeout);
    if (timeout == 0) {
        print("ERROR: TIMEOUT\n");
    } else {
        print("Conversion successful\n");
    }

    // Read back the conversion result
    return opt_adc_read_reg(DATA_ADDR);
}


// reads continuous conversion mode with synchronziation
// make sure operation mode is set to MODE_CONT_CONV before calling

//to do:
// entire read_sync function takes about 10ms (this delay) + 23ms (timeout) ~= 33ms
// need to make settling time of ADC faster
// consider the many filtering options available on ADC
uint32_t opt_adc_read_sync(void){
    //set _SYNC high to begin conversion, resynchronize
    set_cs_high(SYNC_PIN, &SYNC_PORT);
    _delay_ms(10);

    /*FAQ p.10
    "the user can take CS low, initiate the single conversion and then take CS high again...
    When CS is taken high, the DOUT/RDY pin is tristated. Therefore, the DOUT/RDY pin will not indicate the end of the conversion."

    Wait until the conversion finishes, signalled by (DOUT/_RDY_/MISO) going low
    Must set _CS_ low first for this to work
    */

    set_cs_low(CS_PIN, &CS_PORT);

    //wait until conversion finishes, signalled by _RDY going low
    uint16_t timeout = 65535;
    while (bit_is_set(PINB, MISO_PIN) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0){
        print("ERROR: TIMEOUT\n");
    }
    else{
        print("Conversion successful\n");
    }

    print("time: %d\n", timeout);

    //set _SYNC low to resycnhronize
    set_cs_low(SYNC_PIN, &SYNC_PORT);

    return opt_adc_read_reg(DATA_ADDR);
}

// Reads the prints out the values of all registers
// TODO - comment out this function so the constant strings don't waste memory
void opt_adc_read_all_regs(void) {
    uint32_t status = opt_adc_read_reg(STATUS_ADDR);
    uint32_t mode = opt_adc_read_reg(MODE_ADDR);
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);
    uint32_t data = opt_adc_read_reg(DATA_ADDR);
    uint32_t id = opt_adc_read_reg(ID_ADDR);
    uint32_t gpocon = opt_adc_read_reg(GPOCON_ADDR);
    uint32_t offset = opt_adc_read_reg(OFFSET_ADDR);
    uint32_t full_scale = opt_adc_read_reg(FULL_SCALE_ADDR);

    print("\nRegisters:\n");

    print("STATUS: %02lX\n", status);
    print("MODE: %06lX\n", mode);
    print("CONFIG: %06lX\n", config);
    print("DATA: %06lX\n", data);
    print("ID: %02lX\n", id);
    print("GPOCON: %02lX\n", gpocon);
    print("OFFSET: %06lX\n", offset);
    print("FULL_SCALE: %06lX\n", full_scale);
}






// TODO
/*
    // Calibrate the ADC and re-enable continuous conversion mode
    calibrate_adc(SYS_ZERO_SCALE_CALIB);
    calibrate_adc(SYS_FULL_SCALE_CALIB);
    enable_cont_conv_mode();

void opt_adc_calibrate(uint8_t mode_select_bits) {
    // TODO - NOT TESTED YET

    // Read from mode register
    uint32_t mode_data = opt_adc_read_reg(MODE_ADDR);

    // Clear and set mode select bits
    mode_data &= 0xff1fffff;
    mode_data |= ( ((uint32_t) mode_select_bits) << 21 );

    // Write to mode register
    opt_adc_write_reg(MODE_ADDR, mode_data);

    // Check the state of PB0 on the 32M1, which is MISO

    // Wait for _RDY_ to go high (when the calibration starts)
    while (!bit_is_set(PINB, PB0)){
        continue;
    }
    // Wait for _RDY_ to go low (when the calibration finishes)
    while (bit_is_set(PINB, PB0)){
        continue;
    }
}
*/
