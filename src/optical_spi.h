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


// output DATA_RDYn pin (active low)
#define DATA_RDYn       PD7
#define DATA_RDYn_PORT  PORTD
#define DATA_RDYn_DDR   DDRD
#define DATA_RDYn_PIN   PIND


/* SPI OPCODES */
#define CMD_GET_READING             0x01    // 1 cmd byte, followed by 1 byte of well_data
#define CMD_GET_POWER               0x02
#define CMD_ENTER_SLEEP_MODE        0x03
#define CMD_ENTER_NORMAL_MODE       0x04

// test type and field (well) number bits
#define OPT_TYPE_BIT        5
#define FIELD_NUMBER_BIT    4

// number of return bytes
#define SPI_TX_COUNT 3


void init_opt_spi(void);
void opt_set_data_rdy_low();
void opt_set_data_rdy_high();
void opt_loop_main(void);

void manage_cmd (uint8_t spi_first_byte, uint8_t spi_second_byte);
void opt_update_reading(uint8_t well_info);
void opt_transfer_bytes (uint32_t data);

#endif // SPI_COMMS_H
