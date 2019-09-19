#ifndef I2C_MUX_H
#define I2C_MUX_H

#include <i2c/i2c.h>
#include <utilities/utilities.h>
#include <uart/uart.h>

typedef struct {
    uint8_t addr;

    pin_info_t* rst;    
} mux_t;

// 7-bit control byte for accessing the mux (see pg 14)
// Bit[2:0] should be OR'd with the device hardware address
// Does not include r/~w bit
#define MUX_CONTROL_BYTE    0b1110000

/* FUNCTION PROTOTYPES */
void init_mux(mux_t* mux);
void reset_mux(mux_t* mux);
void set_mux_channel(mux_t* mux, uint8_t channel);
uint8_t get_mux_channels(mux_t* mux);
void enable_all_mux_channels(mux_t* mux);
void disable_all_mux_channels(mux_t* mux);

#endif