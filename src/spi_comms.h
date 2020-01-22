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

// slave select pin
#define SS              PB2
#define SS_PORT         PORTB
#define SS_DDR          DDRB
#define SS_PIN          PINB

// output DATA_RDYn pin (active low)
#define DATA_RDYn       PD7
#define DATA_RDYn_PORT  PORTD
#define DATA_RDYn_DDR   DDRD
#define DATA_RDYn_PIN   PIND


#define MAX_COMMAND_QUEUE_LENGTH

/* EXTERNALLY AVAILABLE VARIABLES */
// extern well_t* wells;        already declared extern in optical.h

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
#define CMD_GET_READING             0x01    // 1 cmd byte, followed by 1 byte of well_data


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

void init_spi_comms(void);
uint8_t opt_check_ss_pin(void);
void opt_set_data_rdy_low();
void opt_set_data_rdy_high();
void opt_loop(void);
void manage_cmd(uint8_t cmd_code);
void opt_update_reading(uint8_t well_data);
void opt_transfer_reading();


#endif // SPI_COMMS_H
