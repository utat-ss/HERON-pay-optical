#include "i2c_mux.h"


/*
Initialize the MUX rst
*/
void init_mux(mux_t* mux){
    init_output_pin(mux->rst->pin, mux->rst->ddr, 1);
}

/*
Reset the MUX and downstream I2C busses
See pg 7 of the datasheet for reset timing spec (6 ns)
*/
void reset_mux(mux_t* mux){
    set_pin_low(mux->rst->pin, mux->rst->port);
    _delay_us(1);
    set_pin_high(mux->rst->pin, mux->rst->port);
}

/*
Set the MUX I2C channel
mux: pointer to mux_t
channel: number from 0-7
Only designed to enable one channel at a time
*/
void set_mux_channel(mux_t* mux, uint8_t channel){
    send_start_i2c();
    send_addr_i2c((MUX_CONTROL_BYTE | mux->addr), I2C_WRITE);
    send_data_i2c((uint8_t)(_BV(channel)), I2C_ACK);
    send_stop_i2c();
}

/*
Get the MUX I2C channel register
mux: pointer to mux_t
Bit position of "1"'s in return value corresponds to 
enabled channel(s)
 */
uint8_t get_mux_channels(mux_t* mux){
    uint8_t data; 

    send_start_i2c();
    send_addr_i2c((MUX_CONTROL_BYTE | mux->addr), I2C_READ);
    read_data_i2c(&data, I2C_NACK);
    send_stop_i2c();

    return data;
}

/*
Enable all channels on the specified mux
 */
void enable_all_mux_channels(mux_t* mux){
    send_start_i2c();
    send_addr_i2c((MUX_CONTROL_BYTE | mux->addr), I2C_WRITE);
    send_data_i2c(0xFF, I2C_ACK);
    send_stop_i2c();
}

/*
Disable all channels on the specified mux
Equivalent to resetting the mux, probably slower
 */
void disable_all_mux_channels(mux_t* mux){
    send_start_i2c();
    send_addr_i2c((MUX_CONTROL_BYTE | mux->addr), I2C_WRITE);
    send_data_i2c(0x00, I2C_ACK);
    send_stop_i2c();
}