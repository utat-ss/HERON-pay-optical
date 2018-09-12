#ifndef OPTICAL_ADC_H
#define OPTICAL_ADC_H

#ifndef F_CPU
#define F_CPU 8000000L
#endif

#include <spi/spi.h>
#include <uart/uart.h>
#include <stdint.h>
#include <util/delay.h>


// ADC chip select
#define CS_PIN      PB6
#define CS_PORT     PORTB
#define CS_DDR      DDRB

// Data coming out of the ADC
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
#define CONFIG_PSEUDO       (1UL << 18)
#define CONFIG_UNIPOLAR     (1UL << 3)

#define MODE_SINGLE_CONV    (0b001UL << 21)
#define MODE_POWER_DOWN     (0b011UL << 21)

// Mode select bits for continuous conversion or calibration (p. 21-23, 37)
#define CONT_CONV             0x0
#define INT_ZERO_SCALE_CALIB  0x4
#define INT_FULL_SCALE_CALIB  0x5
#define SYS_ZERO_SCALE_CALIB  0x6
#define SYS_FULL_SCALE_CALIB  0x7


uint32_t opt_adc_read_reg(uint8_t register_addr);
void opt_adc_write_reg(uint8_t register_addr, uint32_t data);

void opt_adc_init(void);

uint32_t opt_adc_read_channel_raw_data(uint8_t channel_num, uint8_t gain);
uint32_t opt_adc_read_field_raw_data(uint8_t field_number);

#endif
