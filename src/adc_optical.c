/*
DESCRIPTION: AD7194 ADC functions
AUTHORS: Bruno Almeida, Dylan Vogel

This is referred to as the "optical ADC", which is different from the primary
ADC library in lib-common.

TODO:
* Consider implementing CS is hold low for conversion process
* Add a function to automatically scale gain
* Add calibration function
* Adapt to handle multiple ADCs
* Fix the CS and hardware address issues

Configuration register bits:
* bit[23] to 0 - chop bit
* bit[20] set to 0 — ref select
* bit[18] should be 1 (psuedo bit)
* bit[16] set to 0 — temp bit
* bit[15:8] – channel select
* bit[7] set to 0 – burnout
* bit[6] set to 0 - ref detect
* bit[4] set to 0 – buffer, might want to enable
* bit[3] set to 1 - unipolar or bipolar (1 for unipolar)
* bit[2:0] set to 000 - programmable gain
*/

#include "adc_optical.h"

void adc_optical_init(void){
    // Initialize ports and registers needed for ADC usage

    init_cs(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_DDR);
    set_cs_high(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_PORT);

    // adc_optical_write_register(CONFIG_ADDR, CONFIG_DEFAULT);
    // "Continuous conversion is the default power-up mode." (p. 32)
}


uint32_t adc_optical_read_register(uint8_t register_addr) {
    // Read the current state of the specified ADC register.

    set_cs_low(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_PORT);
    send_spi(COMM_BYTE_READ_SINGLE | (register_addr << 3));

    // Read the required number of bytes based on register
    uint32_t data = 0;
    for (int i = 0; i < adc_optical_num_register_bytes(register_addr); i++) {
        data = data << 8;
        data = data | send_spi(0);
    }

    // Set CS high
    set_cs_high(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_PORT);

    return data;
}

void adc_optical_write_register(uint8_t register_addr, uint32_t data) {
    // Writes a new state to the specified ADC register.

    set_cs_low(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_PORT);
    send_spi(COMM_BYTE_WRITE | (register_addr << 3));

    // Write the number of bytes in the register
    for (int i = adc_optical_num_register_bytes(register_addr) - 1; i >= 0; i--) {
        send_spi( (uint8_t)(data >> (i * 8)) );
    }

    // Set CS high
    set_cs_high(ADC_OPTICAL_CS_PIN, &ADC_OPTICAL_CS_PORT);
}

uint8_t adc_optical_num_register_bytes(uint8_t register_addr) {
    // Returns the number of BYTES in the specified register at the given address
    // TODO - careful of data register + status information - pg. 20

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

void adc_optical_select_channel(uint8_t channel_num) {
    // Sets the configuration register's bits for the specified ADC input channel.
    // channel_num - one of 5, 6, 7

    // Get the 4 bits for the channel (p. 26)
    uint8_t channel_bits = channel_num - 1;

    // Read the current register status
    uint32_t config_data = adc_optical_read_register(CONFIG_ADDR);

    // Mask the channel bits and write new channel
    config_data &= 0xffff00ff;
    config_data |= (channel_bits << 12);

    // Write modified config register
    adc_optical_write_register(CONFIG_ADDR, config_data);
}

uint32_t adc_optical_read_channel(uint8_t channel_num) {
    // Read 24 bit raw data from the specified ADC channel.
    // channel_num - one of 5, 6, 7

    uint32_t read_data;

    // Select the channel for conversion
    adc_optical_select_channel(channel_num);

    // Wait until the conversion finishes, signalled by MISO going high
    // BUG: conversion is finished when MISO goes *LOW* - fixed
    // TODO: add a timeout
    while (bit_is_set(PINB, PB0)){
        continue;
    }

    // Read back the conversion result
    read_data = adc_optical_read_register(DATA_ADDR);

    return read_data;
}


double adc_optical_convert_reading(uint32_t ADC_reading, uint8_t pga_gain) {
    // Reads the input voltage for the specified ADC channel,
    // including applying the gain factor.
    // channel_num - one of 5, 6, 7


    // (p.31) Code = (2^N * AIN * Gain) / (V_REF)
    //     => AIN = (Code * V_REF) / (2^N * Gain)
    double num = ADC_reading * V_REF;
    double denom = (((uint32_t) 1) << N) * pga_gain;
    double AIN = num / denom;

    // return AIN;
    return AIN;
}


uint8_t adc_optical_convert_gain_bits(uint8_t gain) {
    // Converts the numerical gain to 3 gain select bits (p.25).
    // gain - one of 1, 8, 16, 32, 64, 128

    switch (gain) {
        case 1:
            return 0x00;
            break;
        case 8:
            return 0x03;
            break;
        case 16:
            return 0x04;
            break;
        case 32:
            return 0x05;
            break;
        case 64:
            return 0x06;
            break;
        case 128:
            return 0x07;
            break;
        default:
            return 0x00;
            break;
    }
}

void adc_optical_calibrate(uint8_t mode_select_bits) {
    // TODO - NOT TESTED YET

    // Read from mode register
    uint32_t mode_data = adc_optical_read_register(MODE_ADDR);

    // Clear and set mode select bits
    mode_data &= 0xff1fffff;
    mode_data |= ( ((uint32_t) mode_select_bits) << 21 );

    // Write to mode register
    adc_optical_write_register(MODE_ADDR, mode_data);

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

void adc_optical_enable_cont_conv_mode(void) {
    // Continuous conversion mode

    // Read from mode register
    uint32_t mode_data = adc_optical_read_register(MODE_ADDR);

    // Clear and set mode select bits
    mode_data &= 0xff1fffff;
    mode_data |= ( ((uint32_t) CONT_CONV) << 21 );

    // Write to mode register
    adc_optical_write_register(MODE_ADDR, mode_data);
}

/*void set_PGA(uint8_t gain) {
    // Sets the configuration register's bits for a specified programmable gain.
    // gain - one of 1, 8, 16, 32, 64, 128 (2 and 4 are reserved/unavailable, see p. 25)

    // Convert gain to 3 bits
    uint8_t gain_bits = adc_optical_convert_gain_bits(gain);

    uint8_t gain = 128;

    set_PGA(gain);

    // Write to configuration register
    adc_optical_write_register(CONFIG_ADDR, config_data);

    // Calibrate the ADC and re-enable continuous conversion mode
    calibrate_adc(SYS_ZERO_SCALE_CALIB);
    calibrate_adc(SYS_FULL_SCALE_CALIB);
    enable_cont_conv_mode();
}*/

void adc_optical_set_pga(uint8_t gain) {
    // Sets the configuration register's bits for a specified programmable gain.
    // gain - one of 1, 8, 16, 32, 64, 128 (2 and 4 are reserved/unavailable, see p. 25)

    // Convert gain to 3 bits
    uint8_t gain_bits = adc_optical_convert_gain_bits(gain);

    // Read from configuration register
    uint32_t config_data = adc_optical_read_register(CONFIG_ADDR);

    // Clear gain bits and set
    config_data &= 0xfffffff8;
    config_data |= gain_bits;

    // Write to configuration register
    adc_optical_write_register(CONFIG_ADDR, config_data);
}
