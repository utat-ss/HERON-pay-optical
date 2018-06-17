#ifndef ADC_OPTICAL_H
#define ADC_OPTICAL_H

#include <spi/spi.h>
#include <pex/pex.h>
#include <stdint.h>

#define ADC_OPTICAL_CS_PIN  PB6
#define ADC_OPTICAL_CS_PORT PORTB
#define ADC_OPTICAL_CS_DDR  DDRB

// For writing to the communication register
// (when starting a communication operation with the ADC)
// to specify whether the following operation will be read or write
#define COMM_BYTE_WRITE       0b00000000
#define COMM_BYTE_READ_CONT   0b01000100  // CREAD is set
#define COMM_BYTE_READ_SINGLE 0b01000000  // single conversion
// bit[5:3] is register address
// bit[2] is CREAD. Set to 1 to enable continuous read

// Register addresses (p.20)
#define STATUS_ADDR     0x00
#define MODE_ADDR       0x01
#define CONFIG_ADDR     0x02
#define DATA_ADDR       0x03
#define ID_ADDR         0x04
#define GPOCON_ADDR     0x05
#define OFFSET_ADDR     0x06
#define FULL_SCALE_ADDR 0x07

#define CONFIG_DEFAULT  0x040008

// Current default mode settings are alright.
// #define MODE_DEFAULT    0x00

// For ADC read conversion (p.31)
#define N 24      // number of bits read
#define V_REF 2.5 // reference voltage

// Mode select bits for continuous conversion or calibration (p. 21-23, 37)
#define CONT_CONV             0x0
#define INT_ZERO_SCALE_CALIB  0x4
#define INT_FULL_SCALE_CALIB  0x5
#define SYS_ZERO_SCALE_CALIB  0x6
#define SYS_FULL_SCALE_CALIB  0x7


void adc_optical_init(void);
uint32_t adc_optical_read_register(uint8_t register_addr);
void adc_optical_write_register(uint8_t register_addr, uint32_t data);
uint8_t adc_optical_num_register_bytes(uint8_t register_addr);
void adc_optical_select_channel(uint8_t channel_num);
uint32_t adc_optical_read_channel(uint8_t channel_num);
double adc_optical_convert_reading(uint32_t ADC_reading, uint8_t pga_gain);
uint8_t adc_optical_convert_gain_bits(uint8_t gain);
void adc_optical_calibrate(uint8_t mode_select_bits);
void adc_optical_enable_cont_conv_mode(void);
void adc_optical_set_pga(uint8_t gain);

#endif
