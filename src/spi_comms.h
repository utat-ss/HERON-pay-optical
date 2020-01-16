#ifndef __AVR_ATmega328__ 
#define __AVR_ATmega328__
#endif 

#ifndef SPI_COMMS_H
#define SPI_COMMS_H

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>
#include <stdint.h>
#include "optical.h"
#include "power.h"

#define SS              PB2
#define SS_PORT         PORTB
#define SS_DDR          DDRB
#define SS_PIN          PINB

#define MAX_COMMAND_QUEUE_LENGTH

/* EXTERNALLY AVAILABLE VARIABLES */
extern well_t* wells;

/* function just moves data registers around, so no input or output */
typedef void(*cmd_fn_t)(void);

/* associate desired function with opcode */
typedef struct {
    cmd_fn_t fn;
    uint8_t opcode;
}cmd_t;


/* SPI COMMANDS */
cmd_t update_all_readings_cmd;
cmd_t update_reading_cmd;
cmd_t get_reading_cmd;
cmd_t set_led_bank_cmd;
cmd_t get_led_bank_cmd;
cmd_t enter_sleep_mode_cmd;
cmd_t enter_normal_mode_cmd;
cmd_t get_current_cmd;
cmd_t get_voltage_cmd;
cmd_t get_power_cmd;

/* SPI OPCODES */
#define CMD_UPDATE_ALL_READINGS     0x01
#define CMD_UPDATE_READING          0x02
#define CMD_GET_READING             0x03
#define CMD_SET_LED_BANK            0x04
#define CMD_GET_LED_BANK            0x05
#define CMD_ENTER_SLEEP_MODE        0x06
#define CMD_ENTER_NORMAL_MODE       0x07
#define CMD_GET_CURRENT             0x08
#define CMD_GET_VOLTAGE             0x09
#define CMD_GET_POWER               0x0A

/* SPI STATUS BITS */
#define SPI_DATA_RDY_BIT            7
#define SPI_ERROR_BIT               6
#define SPI_INVALID_COMMAND_BIT     5           



#endif // SPI_COMMS_H
