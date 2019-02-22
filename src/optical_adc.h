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

// _SYNC_ pin
#define SYNC_PIN      PC7
#define SYNC_PORT     PORTC
#define SYNC_DDR      DDRC

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
* bit[18] should be 1 (psuedo bit) for psuedo differential
* bit[16] set to 0 — temp bit
* bit[15:8] – channel select
* bit[7] set to 0 – burnout
* bit[6] set to 0 - ref detect
* bit[4] set to 1 – buffer, might want to enable
* bit[3] set to 1 - unipolar or bipolar (1 for unipolar)
* bit[2:0] set to 000 - programmable gain
*/
#define CONFIG_DEFAULT              0x040008
#define CONFIG_PSEUDO_ON       		  (1UL << 18)
#define CONFIG_PSEUDO_OFF     		  ~(1UL << 18)
#define CONFIG_UNIPOLAR     		    (1UL << 3)
#define CONFIG_BIPOLAR     			    ~(1UL << 3)

// configuration data
#define GPOCON_SETTING				      0b00111000
#define GPOCON_MASK					        0xF0
#define CONFIG_MASK					        0xFFFBFFF0 //clears pseudo, unipolar, and pga
#define PGA_MASK					          0xFFFFF8
#define MODE_MASK					          0x1FFFFF
#define CHANNEL_MASK_POS            0xFFFF0FFF
#define PSEUDO_MASK					        0xFFFBFFFF
#define CHANNEL_MASK_NEG            0xFFFFF0FF
#define REGISTER_ADDRESS_MASK		    0b111

/* Default ADC mode
  bit[23:21] 000 - mode select bits
  bit[20] 0 - disable transmission of status register
  bit[19:18] 10 - use internal clock and tristate MCLK2
  bit[17:16] 00 - disable fast settling
  bit[15] 0 - use sinc4 filter, which gives less noise at higher data rates
  bit[13] 0 - disable parity checking (seems legit)
  bit[12] 0 - disable CLK divide by 2
  bit[11] 1 - enable single cycle settling (slow, but accurate)
  bit[10] 1 - enable 60 hz rejection
  bit[9:0] 0x00 0110 0000 - set FS to 1 for fastest settling time, most noise
*/

#define MODE_DEFAULT                0x080C60

// Operating mode select bits  (p. 21-23, 37)
#define MODE_CONT_CONV              0b000   	// continuous conversion
#define MODE_SINGLE_CONV            0b001   	// single conversion
#define MODE_POWER_DOWN            	0b011		// power down
#define MODE_INT_ZERO_SCALE_CALIB   0b100   	// calibration
#define MODE_INT_FULL_SCALE_CALIB   0b101
#define MODE_SYS_ZERO_SCALE_CALIB   0b110
#define MODE_SYS_FULL_SCALE_CALIB   0b111

void opt_adc_reset(void);
void opt_adc_init_config(void);
void opt_adc_init_mode(void);

void opt_adc_init(void);
void opt_adc_init_sync(uint8_t pair_num);

void opt_adc_select_channel(uint8_t channel_num, uint8_t sign);
void opt_adc_select_pga(uint8_t gain);
void opt_adc_select_op_mode(uint8_t mode_bits);
void opt_adc_select_differential(void);

uint8_t opt_adc_num_reg_bytes(uint8_t register_addr);
uint8_t opt_adc_gain_to_gain_bits(uint8_t gain);

void opt_adc_enable_mux(uint8_t channel);
void opt_adc_disable_mux(void);

uint32_t opt_adc_read_reg(uint8_t register_addr);
void opt_adc_write_reg(uint8_t register_addr, uint32_t data);

uint32_t opt_adc_read_channel_raw_data(uint8_t channel_num, uint8_t gain);
uint32_t opt_adc_read_field_raw_data(uint8_t field_number);
uint32_t opt_adc_read_sync(void);
uint32_t opt_adc_read_cont_conv(void);

void opt_adc_read_all_regs(void);

#endif
