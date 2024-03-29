#ifndef UART_H
#define UART_H

#ifndef __AVR_ATmega328__ 
#define __AVR_ATmega328__
#endif 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


/*
clk_io frequency (p. 17, 48, 54, 143, 280, 282, 298)
With the current fuse settings (FF D7 FF), the CKDIV8 bit is not programmed.
It seems like there is no clock divison for clk_io (therefore 8 MHz).
This is probably better than 1 MHz so we can reach higher UART baud rates.
*/
#define UART_F_IO 8000000UL

// Default baud rate (number of characters per second)
// p. 282, 298
#define UART_DEF_BAUD_RATE 9600

// UART TXD is pin PD3
// UART RXD is pin PD4

// UART RX callback function signature
typedef uint8_t(*uart_rx_cb_t)(const uint8_t*, uint8_t);


// UART RX/TX (from uart.c)
void init_uart(void);
void put_uart_char(uint8_t c);
void get_uart_char(uint8_t* c);
void send_uart(const uint8_t* msg, uint8_t len);
void set_uart_rx_cb(uart_rx_cb_t cb);
uint8_t get_uart_rx_buf_count(void);
void clear_uart_rx_buf(void);

// Printing (from log.c)
int16_t print(char* fmt, ...);
void print_bytes(uint8_t* data, uint16_t len);

#endif // UART_H
