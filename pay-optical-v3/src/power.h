#ifndef __AVR_ATmega8A__ 
#define __AVR_ATmega8A__
#endif 

#ifndef POWER_H
#define POWER_H

#include <stdint.h>
#include <avr/io.h>
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <i2c/i2c.h>
#include "optical.h"


/* POWER DEFINES */
#define POWER_CURR_CHANNEL      0x06
#define POWER_VOLT_CHANNEL      0x07  
#define LOAD_SWITCH_PIN         PB1
#define LOAD_SWITCH_DDR         DDRB
#define LOAD_SWITCH_PORT        PORTB

/* ADC DEFINES */
#define ADC_MUX_MASK        0xF0
#define ADC_VREF_MASK       0x3F
#define ADC_PRESCALER_MASK  0xF8

// AVCC, see page 264
#define ADC_DEF_VREF_BITS   0b01
#define ADC_DEF_VREF        3.3
// Divide f_osc by 64, see page 255 and 267  
#define ADC_DEF_PRESCALER   0b110

/* FUNCTION PROTOTYPES */
void init_power();
void init_board();
void init_board_sensors();
void disable_sensor_power();
void enable_sensor_power();
void enter_sleep_mode();
void enter_normal_mode();
float power_read_current();
float power_read_voltage();
float power_read_power();
float convert_adc_data_to_voltage(uint16_t data, float vref);
void init_adc();
uint16_t read_adc_channel(uint8_t channel);
void set_adc_channel(uint8_t channel);
void set_adc_vref(uint8_t vref);
void set_adc_prescaler(uint8_t prescaler);

#endif