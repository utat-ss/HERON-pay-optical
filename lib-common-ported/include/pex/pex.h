#ifndef PEX_H
#define PEX_H

#ifndef __AVR_ATmega328__ 
#define __AVR_ATmega328__
#endif 

#include <i2c/i2c.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <stdint.h>
#include <avr/io.h>

// Register addresses
// Page 12, table 3-1
#define PEX_IOCON       0x0A  // Assumes bank = 0, such as after reset
// Page 16 of data sheet, table 3-3
#define PEX_IODIR_BASE  0x00 // where direction of each pin is stored.
// Page 18 0 is output, 1 is input
// default is input (Page 1)

#define PEX_GPIO_BASE   0x12 // where GPIO states are stored.

// Register addresses for banks A and B, assuming bank = 0
#define PEX_IODIR_A (PEX_IODIR_BASE)
#define PEX_IODIR_B (PEX_IODIR_BASE + 0x01)
#define PEX_GPIO_A  (PEX_GPIO_BASE)
#define PEX_GPIO_B  (PEX_GPIO_BASE + 0x01)

// Default configuration
#define PEX_IOCON_DEFAULT       0b00001000
// Bit 3 sets hardware addressing

// 7-bit control "byte" for writing and reading registers, see page 15
// Bit[2:0] should be OR'd with the device address
// Does not include r/~w bit, that's handled by I2C
#define PEX_CONTROL_BYTE  0b0100000

// Bits [3:1] are A[2:0] hardware addresses.

// Direction of GPIO pins
typedef enum {
    OUTPUT = 0,
    INPUT = 1
} pex_dir_t;

// Bank A or B of GPIO pins
typedef enum {
    PEX_A = 0,
    PEX_B = 1
} pex_bank_t;

// PEX device
typedef struct {
    uint8_t addr;

    pin_info_t* rst;    
} pex_t;

void init_pex(pex_t*);
void reset_pex(pex_t*);

uint8_t read_pex_register(pex_t*, uint8_t addr);
void write_pex_register(pex_t*, uint8_t addr, uint8_t data);

uint16_t get_pex_bank_pair(pex_t* pex, uint8_t addr);
void set_pex_bank_pair(pex_t* pex, uint8_t addr, uint16_t data);

void set_pex_pin_dir(pex_t* pex, pex_bank_t bank, uint8_t pin, pex_dir_t dir);
void set_pex_pin(pex_t* pex, pex_bank_t bank, uint8_t pin, uint8_t state);
uint8_t get_pex_pin(pex_t* pex, pex_bank_t bank, uint8_t pin);

#endif // PEX_H
