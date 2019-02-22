#ifndef SYNCDEMOD_H
#define SYNCDEMOD_H

#include <uart/uart.h>
#include <stdint.h>
#include <spi/spi.h>
#include <utilities/utilities.h>

#define SD_CS_PORT  PORTC
#define SD_CS_DDR   DDRC
#define SD1_CS_PIN  PC3
#define SD2_CS_PIN  PC4
#define SD3_CS_PIN  PC5
#define SD4_CS_PIN  PC6

#define SD_SERIAL_CONFIG_ADDR       0x0000
#define SD_FILTER_STROBE_ADDR       0x0010
#define SD_FILTER_CONFIG_BASE_ADDR  0x0011
#define SD_ANALOG_CONFIG_ADDR       0x0028
#define SD_SYNC_CONTROL_ADDR        0x0029
#define SD_DEMOD_CONTROL_ADDR       0x002A
#define SD_CLK_CONFIG_ADDR          0x002B
#define SD_DIGITAL_CONFIG_ADDR      0x002C
#define SD_CORE_RESET_ADDR          0x002D

// No reset, MSB first, addr increment, 3-wire SPI
#define SD_SERIAL_CONFIG_DEFAULT    0x00

// bit 6 is phase of RCLK, bit 4 is mixer enable, bit 3 is SDO/RCLK routing, bit[2:0] is VOCM config
// bit 6 == 1 delays phase of RCLK
// bit 4 == 1 holds the last sample while RCLK inactive
// bit 3 == 1 sends RCLK to output
// bit[2:0] == 010 sets VOCM to VDD/2 fast settling mode
#define SD_DEMOD_CONTROL_DEFAULT    0x1A

// default edge position of the synco output, see fig 24 on pg 14 of the datasheet
#define SD_SYNCO_EDGE_DEFAULT          0b1101
// default polarity is non-inverted
#define SD_SYNCO_POLARITY_DEFAULT      0

// define function pointer type
typedef void(*sd_fn_t)(void);

void syncdemod_init();
void syncdemod_write_register(uint8_t sd_cs_pin, uint16_t addr, uint8_t data);
void syncdemod_write_multibyte(uint8_t sd_cs_pin, uint16_t addr, uint8_t len, uint8_t* data);
void syncdemod_set_filters(uint8_t sd_cs_pin, uint8_t* config_arr);
void syncdemod_set_clock(uint8_t sd_cs_pin, int clkin_div, uint8_t rclk_div);
void syncdemod_write_synco(uint8_t sd_cs_pin, uint8_t enable, uint8_t polarity, uint8_t edge);
void syncdemod_enable_external_clk(uint8_t sd_cs_pin);
void syncdemod_disable_external_clk(uint8_t sd_cs_pin);
void syncdemod_enable_rclk(uint8_t sd_cs_pin);
void syncdemod_disable_rclk(uint8_t sd_cs_pin);
void syncdemod_reset(uint8_t sd_cs_pin);
void syncdemod_toggle_core_reset(uint8_t sd_cs_pin);
void syncdemod_init_interrupt(sd_fn_t f0, sd_fn_t f1, sd_fn_t f2, sd_fn_t f3);

#endif
