/*
    Library for using I2C (TWI) on the Atmega8a
    I2C has a lot of potential states during use, so the library is written such
    that the application programmer can decide how to handle the I2C state machine in their code.

    Written by: Dylan Vogel

    Example use (not necessarily good use, no error handling here):

        int main(void){
            uint8_t dummy_addr;
            uint8_t dummy_data;

            init_i2c();
            send_start_i2c();
            send_addr_i2c(dummy_addr, I2C_WRITE);
            send_data_i2c(dummy_data, I2C_ACK);
            send_stop_i2c();
        }
*/

#include <i2c/i2c.h>

/*
Initializes I2C (TWI) at 400k (default) w/out interrupts
 */
void init_i2c(void){
    TWBR = I2C_DEF_BITRATE;                             // set bit rate
    TWSR |= (~I2C_PRESCALER_MASK & I2C_DEF_PRESCALER);  // set prescaler
    TWCR |= _BV(TWEN);                                  // enable I2C. somewhat irrelevant since it's set in every other function
}

/*
Request to send a START condition on the I2C bus
*/
uint8_t send_start_i2c(void){
    uint16_t timeout = UINT16_MAX;

    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);         // request to send a START condition on the bus
    while(!(TWCR & _BV(TWINT)) && timeout--);
    if (!timeout) return 1;                             // return timeout error
    if (((TWSR & I2C_PRESCALER_MASK) != I2C_START) || ((TWSR & I2C_PRESCALER_MASK) != I2C_RSTART)){
        return (TWSR & I2C_PRESCALER_MASK);             // return status that generated error
    }
    return 0;
}


/*
Writes an address onto the I2C bus as master
addr: 7 bit address
read_or_write: read/~write
*/
uint8_t send_addr_i2c(uint8_t addr, uint8_t read_or_write){
    uint16_t timeout = UINT16_MAX;
    uint8_t slave_addr = (addr << 1) | read_or_write;
    uint8_t status = 0;

    // set expected return status
    switch (read_or_write){
        case I2C_READ:
            status = I2C_SLAR_ACK;
            break;
        case I2C_WRITE:
            status = I2C_SLAW_ACK;
            break;
    }

    TWDR = slave_addr;                          // load slave address
    TWCR = _BV(TWINT) | _BV(TWEN);              // clear int flag to transmit data register
    while(!(TWCR & _BV(TWINT)) && timeout--);
    if (!timeout) return 1;                     // return timeout error
    if ((TWSR & I2C_PRESCALER_MASK) != status){
        return (TWSR & I2C_PRESCALER_MASK);     // return status that generated error
    }
    return 0;
}

/*
Writes a data byte to I2C bus as master
data: data to send
ack: expect ack/~nack
*/
uint8_t send_data_i2c(uint8_t data, uint8_t ack){
    uint16_t timeout = UINT16_MAX;
    uint8_t status = 0;

    // set expected return status
    switch (ack){
        case I2C_ACK:
            status = I2C_DATA_ACK;
            break;
        case I2C_NACK:
            status = I2C_DATA_NACK;
            break;
    }

    TWDR = data;                                // load data to send
    TWCR = _BV(TWINT) | _BV(TWEN);              // clear int flag to transmit data register        
    while(!(TWCR & _BV(TWINT)) && timeout--);
    if (!timeout) return 1;                     // return timeout error
    if ((TWSR & I2C_PRESCALER_MASK) != status){
        return (TWSR & I2C_PRESCALER_MASK);     // return status that generated error
    }
    return 0;

}

/*
Receives a data byte from the I2C bus as master
data: pointer to where data will be stored (return is used for error handling)
ack: send ack/~nack, NACK indicates end of transmission to the slave
*/
uint8_t read_data_i2c(uint8_t* data, uint8_t ack){
    uint16_t timeout = UINT16_MAX;
    uint8_t status = 0;

    // set expected return status
    switch (ack){
        case I2C_ACK:
            status = I2C_RDATA_ACK;
            break;
        case I2C_NACK:
            status = I2C_RDATA_NACK;
            break;
    }

    TWCR = _BV(TWINT) | (ack << TWEA) | _BV(TWEN);         // request data and return ack/~nack
    while(!(TWCR & _BV(TWINT)) && timeout--);
    if(!timeout) return 1;
    if ((TWSR & I2C_PRESCALER_MASK) != status){
        *data = TWDR;                           // might not be correct, we'll see
        return (TWSR & I2C_PRESCALER_MASK);     // return status that generated error
    }
    *data = TWDR;
    return 0;
}

/*
Sends a STOP condition on the I2C bus
*/
uint8_t send_stop_i2c(void){
    uint16_t timeout = UINT16_MAX;

    TWCR = _BV(TWINT) | _BV(TWSTO) |_BV(TWEN);  // request to send stop condition
    while((TWCR & _BV(TWSTO)) && timeout--);    // wait for TWSTO flag to be cleared
    if(!timeout) return 1;                      // timeout error
    return 0;                                   // STOP condition was sent
}

/*
"Handles" an I2C error
*/
void handle_error_i2c(uint8_t status_code){
    switch (status_code){
        case I2C_BUS_ERROR:
            TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);    // clears an I2C bus error
            break;
        default:
            print("I2C error occured with code: 0x%02X", status_code);
            break;
    }
}