#ifndef OPTICAL_H
#define OPTICAL_H

#include <pex/pex.h>
#include <stdint.h>
#include <i2c/i2c.h>

/* PORT EXPANDER ADDRESSES (HARDWARE) */
#define OPTICAL_PEX1_ADDR       0b001
#define OPTICAL_PEX2_ADDR       0b010
#define LED_PEX1_ADDR           0b011
#define LED_PEX2_ADDR           0b100

/* I2C MUX ADDRESSES (HARDWARE) */
#define I2C_MUX1_ADDR           0b000
#define I2C_MUX2_ADDR           0b001
#define I2C_MUX3_ADDR           0b010
#define I2C_MUX4_ADDR           0b011

/* QUALITY OF LIFE DEFINES */
typedef enum {
    PAY_OPTICAL,
    PAY_LED
} pay_board_t;

typedef enum {
    LED_ON  = 1,
    LED_OFF = 0
} led_state_t;

/* To be implemented:

void enter_low_power(void);
void exit_low_power(void)
uint32_t get_power_info(void);
uint16_t get_sensor_data(uint8_t pos);

 */

/* FUNCTION PROTOTYPES */
void all_on(void);
void all_off(void);
void init_all_pex(void);
void init_opt_pex(void);
void init_led_pex(void);
void set_led(uint8_t pos, pay_board_t board, led_state_t state);
uint8_t get_led(uint8_t pos, pay_board_t board);

#endif