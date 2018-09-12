/*
DESCRIPTION: AD7194 ADC functions
AUTHORS: Bruno Almeida, Dylan Vogel

Datasheet: http://www.analog.com/media/en/technical-documentation/data-sheets/AD7194.pdf
FAQ: http://www.analog.com/media/en/technical-documentation/frequently-asked-questions/AD719x_FAQ_Instru_Conv.pdf

This is referred to as the "optical ADC", which is different from the primary
ADC library in lib-common.

- single conversion mode

TODO:
* Add a function to automatically scale gain
* Add calibration function
* Use 4 GPIO pins for mux A0, A1, A2, EN
* Switch GPIO outputs to mux when appropriate, create function
*/

#include "optical_adc.h"


void opt_adc_init_config(void);
void opt_adc_init_mode(void);

void opt_adc_select_channel(uint8_t channel_num);
void opt_adc_select_pga(uint8_t gain);
void opt_adc_select_op_mode(uint8_t mode_bits);

void opt_adc_enable_mux(uint8_t channel);
void opt_adc_disable_mux(void);

uint8_t opt_adc_num_reg_bytes(uint8_t register_addr);
uint8_t opt_adc_gain_to_gain_bits(uint8_t gain);



// TODO - p.32 - "Following a reset, the user should allow a period of 200 μs before addressing the serial interface."
void opt_adc_init(void){
    // Initialize ports and registers needed for ADC usage

    init_cs(CS_PIN, &CS_DDR);
    set_cs_high(CS_PIN, &CS_PORT);

    // opt_adc_write_reg(CONFIG_ADDR, CONFIG_DEFAULT);
    // "Continuous conversion is the default power-up mode." (p. 32)

    // TODO - start in power down mode

    // GPOCON register - enable 4 GPIO outputs
    // _EN_ = 1, A2/A1/A0 = 0
    // TODO - make a constant?
    opt_adc_write_reg(GPOCON_ADDR, 0b00111000);

    opt_adc_init_config();
    opt_adc_init_mode();
}


// Initializes configuration register
void opt_adc_init_config(void) {
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);

    // Enable pseudo-differential output
    // Enable unipolar (positive voltage) mode
    // Set gain to 0b000 (PGA = 1)
    // Pseudo bit = 1
    config = config | CONFIG_PSEUDO;
    config = config | CONFIG_UNIPOLAR;
    config = config & 0xFFFFF8;

    opt_adc_write_reg(CONFIG_ADDR, config);
}


// Initializes mode register
void opt_adc_init_mode(void) {
    uint32_t mode = opt_adc_read_reg(MODE_ADDR);

    // Clear first 3 bits and set operating mode to power down mode
    mode = mode & 0x1FFFFF;
    mode = mode | MODE_POWER_DOWN;

    opt_adc_write_reg(MODE_ADDR, mode);
}


// TODO - is the _RDY_ pin only set low for a read from the data register or any register?
uint32_t opt_adc_read_reg(uint8_t register_addr) {
    // Read the current state of the specified ADC register.
    print("\n");
    register_addr = register_addr & 0b111;

    // Start communication
    set_cs_low(CS_PIN, &CS_PORT);

    // Begin communication with a write to the communications register
    // Request a read from the specified register
    uint8_t spi_tx = COMM_READ | (register_addr << 3);
    send_spi(spi_tx);
    print("Read register request: %02X\n", spi_tx);

    // Read the required number of bytes based on register
    uint32_t data = 0;
    uint8_t num_bytes = opt_adc_num_reg_bytes(register_addr);
    for (uint8_t i = 0; i < num_bytes; i++) {
        uint8_t next_byte = send_spi(0x00);
        print("Received byte: %02X\n", next_byte);
        data = (data << 8) | next_byte;
    }

    // Stop communication
    set_cs_high(CS_PIN, &CS_PORT);

    print("Read register data: %06lX\n", data);

    return data;
}


void opt_adc_write_reg(uint8_t register_addr, uint32_t data) {
    // Writes a new state to the specified ADC register.
    print("\n");
    register_addr = register_addr & 0b111;

    // Start communication
    set_cs_low(CS_PIN, &CS_PORT);

    // Begin communication with a write to the communications register
    // Request a write to the specified register
    uint8_t spi_tx = COMM_WRITE | (register_addr << 3);
    send_spi(spi_tx);
    print("Write register request: %02X\n", spi_tx);

    print("Write register data: %06lX\n", data);

    // Write the number of bytes in the register
    for (int8_t i = opt_adc_num_reg_bytes(register_addr) - 1; i >= 0; i--) {
        uint8_t next_byte = data >> (i * 8);
        send_spi(next_byte);
        print("Sent byte: %02X\n", next_byte);
    }

    // Set CS high
    set_cs_high(CS_PIN, &CS_PORT);
}


void opt_adc_select_channel(uint8_t channel_num) {
    // Sets the configuration register's bits for the specified ADC input channel.
    // channel_num must be between 1 and 16

    // Get the 4 bits for the channel for psuedo-differential positive input (p. 26)
    uint8_t channel_bits = (channel_num - 1) << 4;

    // Read the current register status
    uint32_t config = opt_adc_read_reg(CONFIG_ADDR);

    // Mask the channel bits and write new channel
    config &= 0xffff00ff;
    config |= ((uint32_t) channel_bits) << 8;

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
    config_data &= 0xfffff8;
    config_data |= gain_bits;

    // Write to configuration register
    opt_adc_write_reg(CONFIG_ADDR, config_data);
}


// Selects the operating mode
void opt_adc_select_op_mode(uint8_t mode_bits) {
    // Read from mode register
    uint32_t mode_reg = opt_adc_read_reg(MODE_ADDR);

    // Clear mode bits and set
    mode_reg &= 0x1fffff;
    mode_reg |= ((uint32_t) mode_bits) << 21;

    // Write to configuration register
    opt_adc_write_reg(MODE_ADDR, mode_reg);
}


// See the single conversion mode, p.33
// TODO - CS must be kept low the whole time
// (write to mode register, then read from data register)
// TODO - read_reg and write_reg functions should not set CS
// TODO - p.23 - "The internal clock requires 200 μs typically to power up and settle."
uint32_t opt_adc_read_channel_raw_data(uint8_t channel_num, uint8_t gain) {
    // Reads 24 bit raw data from the specified ADC channel.

    // Select the channel for conversion
    opt_adc_select_channel(channel_num);
    opt_adc_select_pga(gain);
    opt_adc_select_op_mode(MODE_SINGLE_CONV);

    // p.23 - "The internal clock requires 200 μs typically to power up and settle."
    // TODO - check delay time - is it necessary?
    _delay_us(200);

    /*
    FAQ p.10
    "the user can take CS low, initiate the single conversion and then take CS high again...
    When CS is taken high, the DOUT/RDY pin is tristated. Therefore, the DOUT/RDY pin will not indicate the end of the conversion."
    */
    // Wait until the conversion finishes, signalled by (DOUT/_RDY_/MISO) going low
    // Must set _CS_ low first for this to work
    set_cs_low(CS_PIN, &CS_PORT);
    // TODO - change timeout
    uint16_t timeout = 65535;
    while (bit_is_set(PINB, MISO_PIN) && timeout > 0) {
        timeout--;
    }
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


// channel - channel to select (between 1 and 8 to represent S1-S8)
void opt_adc_enable_mux(uint8_t channel) {
    uint8_t channel_bits = channel - 1;

    // Set _EN_ (bit 3) = 0, bits 2-0 = channel
    uint8_t gpocon = opt_adc_read_reg(GPOCON_ADDR);
    gpocon = gpocon & 0xF0;
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

    // Enable the mux for the appropriate address
    // (this should turn on the LED and enable the amplifier)
    opt_adc_enable_mux(address + 1);
    _delay_ms(100);
    // TODO - set up and configure synchronous demodulator

    uint8_t adc_channel;
    switch (group) {
        case 0:
            adc_channel = 5;
            break;
        case 1:
            adc_channel = 7;
            break;
        case 2:
            adc_channel = 9;
            break;
        case 3:
            adc_channel = 11;
            break;
        case 4:
            adc_channel = 13;
            break;
        default:
            print("Unexpected sensor group\n");
            adc_channel = 5;
            break;
    }

    // Read ADC data and prepare to send it over SPI
    uint32_t raw_data = opt_adc_read_channel_raw_data(adc_channel, 1);
    opt_adc_disable_mux();

    return raw_data;
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

    print("Registers:\n");

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
