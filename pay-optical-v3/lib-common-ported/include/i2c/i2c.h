#ifndef __AVR_ATmega8A__ 
#define __AVR_ATmega8A__
#endif 

#ifndef I2C_H
#define I2C_H

#include <utilities/utilities.h>
#include <uart/uart.h>

#define I2C_DEF_BITRATE     2       // 400k baud if prescaler = 1
#define I2C_DEF_PRESCALER   0x00    // prescaler = 1
#define I2C_PRESCALER_MASK  0xF8    // Mask the prescaler bits when checking TWSR

#define I2C_READ            0x01    // Read is 1
#define I2C_WRITE           0x00    // Write is 0

#define I2C_ACK             0x01    // ACK is 1
#define I2C_NACK            0x00    // NACK is 0

/*  STATUS CODE DEFINES */
#define I2C_START           0x08    // Start has been transmitted
#define I2C_RSTART          0x10    // Repeated start has been transmitted
#define I2C_ARB_LOST        0x38    // Arbitration has been lost
#define I2C_BUS_ERROR       0x00    // Bus error has occurred

// Writing to I2C bus
#define I2C_SLAW_ACK        0x18    // SLA+W transmitted, ACK received
#define I2C_SLAW_NACK       0x20    // SLA+W transmitted, NACK received
#define I2C_DATA_ACK        0x28    // Data byte transmitted, ACK received
#define I2C_DATA_NACK       0x30    // Data byte transmitted, NACK received

// Reading from I2C bus
#define I2C_SLAR_ACK        0x40    // SLA+R transmitted, ACK received
#define I2C_SLAR_NACK       0x48    // SLA+R transmitted, NACK received
#define I2C_RDATA_ACK       0x50    // Data byte received, ACK returned
#define I2C_RDATA_NACK      0x58    // Data byte received, NACK returned

void init_i2c(void);
uint8_t send_start_i2c(void);
uint8_t send_addr_i2c(uint8_t addr, uint8_t read_or_write);
uint8_t send_data_i2c(uint8_t data, uint8_t ack);
uint8_t read_data_i2c(uint8_t* data, uint8_t ack);
uint8_t send_stop_i2c(void);
void handle_error_i2c(uint8_t status_code);

#endif // I2C_H