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

/* SPI OPCODES */
#define CMD_GET_READING             0x01    // 1 cmd byte, followed by 1 byte of well_data
#define CMD_GET_POWER               0x02

// unimplemented
#define CMD_SET_LED_BANK            0x04
#define CMD_GET_LED_BANK            0x05
#define CMD_ENTER_SLEEP_MODE        0x06
#define CMD_ENTER_NORMAL_MODE       0x07

// number of return bytes
#define NUM_GET_READING             3
#define NUM_GET_POWER               3

/* SPI STATUS BITS */
#define SPI_ERROR_BIT               6
#define SPI_INVALID_COMMAND_BIT     5           

// test type and field (well) number bits
#define TEST_TYPE_BIT        5
#define FIELD_NUMBER_BIT       4

void init_spi_comms(void);
uint8_t opt_check_ss_pin(void);
void opt_set_data_rdy_low();
void opt_set_data_rdy_high();
void opt_loop(void);

void manage_cmd (uint8_t spi_first_byte, uint8_t spi_second_byte);
void opt_update_reading(uint8_t well_info);
void opt_transfer_reading(uint8_t well_info);
void opt_transfer_bytes (uint32_t data, uint8_t num_bytes);

#endif // SPI_COMMS_H
