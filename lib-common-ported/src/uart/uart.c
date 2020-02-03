/*
UART (TX and RX) library.

32M1/64M1 datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8209-8-bit%20AVR%20ATmega16M1-32M1-64M1_Datasheet.pdf

UART is a protocol that allows devices to send data to each
other one byte at a time (usually represented as one character in a
user-friendly terminal).
*/

#include <uart/uart.h>
#include <string.h>

// Maximum number of characters the UART RX buffer can store
#define UART_MAX_RX_BUF_SIZE    50
#define UBRR    (uint16_t)(UART_F_IO/16/UART_DEF_BAUD_RATE - 1)

// Buffer of received characters
volatile uint8_t uart_rx_buf[UART_MAX_RX_BUF_SIZE];
// Number of valid characters in buffer (starting at index 0)
volatile uint8_t uart_rx_buf_count;

// default rx callback (no operation)
uint8_t _uart_rx_cb_nop(const uint8_t* c, uint8_t len) {
    return 0;
}
// Global RX callback function
uart_rx_cb_t uart_rx_cb = _uart_rx_cb_nop;


/*
Initializes the UART library
No parity bit, 8 bit data, 9600 baud
 */ 
void init_uart(void) {
    // disable interrupts
    cli();
    // Set the bit rate register
    UBRR0H = (uint8_t)(UBRR >> 8);
    UBRR0L = (uint8_t)UBRR;
    // set doublespeed mode; not needed for 9600 baud
    // UCSR0A |= _BV(U2X);
    // Enable RX, TX and RX interrupts
    UCSR0B |= _BV(TXEN0) | _BV(RXEN0);
    // Set UART to use 8 data bits
    UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00);

    // reset RX buffer and counter
    clear_uart_rx_buf();
    // Set default (no operation) RX callback
    uart_rx_cb = _uart_rx_cb_nop;

    // globally enable interrupts
    sei();
}

/*
Sends one character over UART (TX)
Waits for the UDRE bit to go high to indicate that data can be written
c - character to send
*/
void put_uart_char(uint8_t c) {
    uint16_t timeout = UINT16_MAX;
    while (!(UCSR0A & _BV(UDRE0)) && timeout--);
    UDR0 = c;
}

/*
Gets one (received) character from UART (RX).
c - will be set by this function to the received character
*/
void get_uart_char(uint8_t* c) {
    uint16_t timeout = UINT16_MAX;
    while (!(UCSR0A & _BV(RXC0)) && timeout--);
    *c = UDR0;
}

/*
Sends a sequence of characters over UART.
msg - pointer to start of array
len - number of characters
*/
void send_uart(const uint8_t* msg, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        put_uart_char(msg[i]);
    }
}

/*
Sets the callback function that will be called when UART receives data.
cb - callback function

The function must have the following signature:
uint8_t func(const uint8_t* data, uint8_t len);

The UART library will pass a pointer to the array (data) and the number of characters (len).
The function can process the characters however it wants and must return the
number of characters it has "processed", which will then be removed from the
buffer of received UART characters.
*/
void set_uart_rx_cb(uart_rx_cb_t cb) {
    uart_rx_cb = cb;
}

/*
Gets the number of characters that are currently in the UART RX buffer but have
not been processed yet.
*/
uint8_t get_uart_rx_buf_count(void) {
    return uart_rx_buf_count;
}

/*
Clears the RX buffer (sets all values in the array to 0, sets counter to 0).
*/
void clear_uart_rx_buf(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        uart_rx_buf_count = 0;
        for (uint8_t i = 0; i < UART_MAX_RX_BUF_SIZE; i++) {
            uart_rx_buf[i] = 0;
        }
    }
}

// Interrupt handler that will be called when we receive a character over UART
ISR(USART_RX_vect) {
    // Check if we got the interrupt for a received character (p. 293)
    if (UCSR0A & _BV(RXC0)) {
        // Fetch the new recieved character
        static uint8_t c;
        // reading the UDR also clears the RXC interrupt flag
        get_uart_char(&c);

        // Add the new character to the RX buffer
        uart_rx_buf[uart_rx_buf_count] = c;
        uart_rx_buf_count += 1;

        /*
        Call the RX callback function to process the character buffer
        It's fine to cast the buffer pointer to non-volatile, because we are in
        an interrupt handler so the callback function can't be interrupted
        (i.e. the contents of uart_rx_buf can't change)
        */
        uint8_t read_bytes = uart_rx_cb(
            (const uint8_t*) uart_rx_buf, uart_rx_buf_count);

        // If some number of bytes were read, shift everything in the buffer
        // leftward by the number of bytes read
        if (read_bytes > 0) {
            uart_rx_buf_count -= read_bytes;

            if (uart_rx_buf_count > 0) {
                // parameters - destination, source, number of bytes
                memmove((void*) uart_rx_buf, (void*) (uart_rx_buf + read_bytes),
                    uart_rx_buf_count);
            }
        }

        // If the buffer is full, clear it
        if (uart_rx_buf_count >= UART_MAX_RX_BUF_SIZE) {
            clear_uart_rx_buf();
        }
    }
}
