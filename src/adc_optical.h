#ifndef ADC_OPTICAL_H
#define ADC_OPTICAL_H

#include <spi/spi.h>
#include <stdint.h>




#define ADC_OPTICAL_CS_PIN  PB6
#define ADC_OPTICAL_CS_PORT PORTB
#define ADC_OPTICAL_CS_DDR  DDRB

#define MISO_PIN    PB0
#define MISO_PORT   PORTB
#define MISO_DDR    DDRB



// Register addresses (p.20)
#define STATUS_ADDR     0x00
#define MODE_ADDR       0x01
#define CONFIG_ADDR     0x02
#define DATA_ADDR       0x03
#define ID_ADDR         0x04
#define GPOCON_ADDR     0x05
#define OFFSET_ADDR     0x06
#define FULL_SCALE_ADDR 0x07

// Communications register
#define COMM_WRITE  (0 << 6)
#define COMM_READ   (1 << 6)

/*
TODO - check configuration register bits:
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
#define CONFIG_PSEUDO   ( ((uint32_t) 1) << 18 )
#define CONFIG_UNIPOLAR ( ((uint32_t) 1) << 3  )

#define MODE_SINGLE_CONV    ( ((uint32_t) 0b001) << 21 )
#define MODE_POWER_DOWN     ( ((uint32_t) 0b011) << 21 )




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
void adc_optical_init_config(void);
void adc_optical_init_mode(void);

uint32_t adc_optical_read_register(uint8_t register_addr);
void adc_optical_write_register(uint8_t register_addr, uint32_t data);

void adc_optical_select_channel(uint8_t channel_num);
void adc_optical_select_pga(uint8_t gain);
void adc_optical_select_operating_mode(uint32_t mode_bits);

uint32_t adc_optical_read_raw_data(uint8_t channel_num, uint8_t gain);
double adc_optical_convert_raw_data_to_voltage(uint32_t raw_data, uint8_t gain);

uint8_t adc_optical_num_register_bytes(uint8_t register_addr);
uint8_t adc_optical_convert_gain_to_gain_bits(uint8_t gain);

#endif
