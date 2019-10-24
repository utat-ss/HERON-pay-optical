#ifndef OPTICAL_H
#define OPTICAL_H

#include <pex/pex.h>
#include <stdint.h>
#include <uart/uart.h>
#include <i2c/i2c.h>
#include "i2c_mux.h"
#include "light_sens.h"

/* PORT EXPANDER ADDRESSES (HARDWARE) */
#define OPTICAL_PEX1_ADDR       0b001
#define OPTICAL_PEX2_ADDR       0b010
#define LED_PEX1_ADDR           0b011
#define LED_PEX2_ADDR           0b100

/* I2C MUX ADDRESSES (HARDWARE) */
#define I2C_MUX1_ADDR           0b000
#define I2C_MUX2_ADDR           0b011
#define I2C_MUX3_ADDR           0b010
#define I2C_MUX4_ADDR           0b100

/* CALIBRATION DEFINES */
// hysteresis thresholds, to stop it from calibrating when it's at the edge
#define OPT_SENS_HYST_LOW_THRES      0.05
#define OPT_SENS_HYST_HIGH_THRES     0.95

#define OPT_SENS_LOW_THRES           0.1
#define OPT_SENS_HIGH_THRES          0.9

/* QUALITY OF LIFE DEFINES */
typedef enum {
    PAY_OPTICAL = 0,
    PAY_LED = 1
} pay_board_t;

typedef enum {
    LED_ON  = 1,
    LED_OFF = 0
} led_state_t;

typedef enum {
    FLUOR_MEAS = 0,
    OD_MEAS = 1
} meas_type_t;

/* To be implemented:

void enter_low_power(void);
void exit_low_power(void)
uint32_t get_power_info(void);
uint16_t get_sensor_data(uint8_t pos);

 */

/* FUNCTION PROTOTYPES */
void init_opt_sensors(void);
uint32_t get_opt_sensor_reading(uint8_t pos, pay_board_t board);
void calibrate_opt_sensor_sensitivity(light_sensor_t* light_sens);
void all_on();
void all_off();
void init_all_mux(void);
void init_all_pex(void);
void init_pex_output_low(pex_t* pex);
void set_led(uint8_t pos, pay_board_t board, led_state_t state);
uint8_t get_led(uint8_t pos, pay_board_t board);
void get_pex(pex_t** pex, uint8_t pos, pay_board_t board);
uint8_t get_mux(mux_t** mux, uint8_t pos);

#endif