/*
MCP23017 port expander (PEX)
Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf

A port expander is a device with many GPIO (general purpose input/output) pins.
Each GPIO pin can function as either an input or an output, depending on what
you want to use it for. Using a port expander gives us more GPIO pins to work
with.

The micro communicates with the PEX over I2C, telling it to use pins as inputs or
outputs, read values from pins, or write values to pins.

The PEX has two "banks", labelled A and B. Each has 8 pins,
labelled GPA0-GPA7 and GPB0-GPB7.

Each PEX device has a 3-bit address, determined by how the pins A2, A1, A0
are connected in hardware. Each I2C command contains the address of the PEX it
is meant for, allowing up to 8 PEX devices to be connected using the same I2C bus. A PEX will only respond to the command if its address
matches the one sent in the comand.

AUTHORS: Dylan Vogel, Shimi Smith, Bruno Almeida, Siddharth Mahendraker
*/

#include <pex/pex.h>

/*
Initializes port expander reset and chip select pins on the 32M1
pex - pointer to the pex device
*/
void init_pex(pex_t* pex) {
    // Init RST if it's enabled
    if(pex->rst != NULL){
        init_cs(pex->rst->pin, pex->rst->ddr);
    }
    // Default configuration
    write_pex_register(pex, PEX_IOCON, PEX_IOCON_DEFAULT);
}

/*
Resets the port expander
pex - pointer to the pex device
*/
void reset_pex(pex_t* pex) {
    // Reset the port expander if it's enabled
    if (pex->rst != NULL){
        set_cs_low(pex->rst->pin, pex->rst->port);
        _delay_ms(1); // minimum 1 microsecond
        set_cs_high(pex->rst->pin, pex->rst->port);
        _delay_ms(1);
    } else {
        print("Error: RST not connected on PEX");
    }

}

 /*
 Writes data to register
 pex - pointer to the pex device
 addr - address of register being written to
 data - 8 bit data to write to the registers
 */
void write_pex_register(pex_t* pex, uint8_t addr, uint8_t data) {
    // I2C control byte format: pg 15
    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_WRITE);
    send_data_i2c(addr, I2C_ACK);
    send_data_i2c(data, I2C_ACK);
    send_stop_i2c();
}

/*
Reads data from register 'addr'
e.g. if 'addr' = PEX_IODIR_A then IO data for bank A is returned
e.g. if 'addr' = PEX_GPIO_A then the values of bank A is returned
pex - pointer to the pex device
addr - address of register to read
*/
uint8_t read_pex_register(pex_t* pex, uint8_t addr) {
    uint8_t data = 0;

    // I2C control byte format: pg 15
    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_WRITE);
    send_data_i2c(addr, I2C_ACK);
    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_READ);
    read_data_i2c(&data, I2C_NACK);
    send_stop_i2c();

    return data;
}

/*
Returns a read of bank A and B for a particular address pair
Assumes the bank A address is given
High byte return is bank B, low byte is bank A, MSB first
*/
uint16_t get_pex_bank_pair(pex_t* pex, uint8_t addr){
    // See page 13 for a description of this mode

    uint8_t reth = 0;
    uint8_t retl = 0;

    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_WRITE);
    send_data_i2c(addr, I2C_ACK);
    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_READ);
    read_data_i2c(&retl, I2C_ACK);
    read_data_i2c(&reth, I2C_NACK);
    send_stop_i2c();

    print("reth shifted is: 0x%04X\n", (reth << 8));
    print("retl is: 0x%02X\n", retl);

    return (uint16_t)((reth << 8) | retl);
}

/*
Writes to both bank A and B for a particular address pair
Assumes bank A address is given
High byte of data is bank B, low byte is bank A
*/
void set_pex_bank_pair(pex_t* pex, uint8_t addr, uint16_t data){
    // See page 13 for a description of this mode

    send_start_i2c();
    send_addr_i2c((PEX_CONTROL_BYTE | (pex->addr)), I2C_WRITE);
    send_data_i2c(addr, I2C_ACK);
    send_data_i2c((uint8_t)(data), I2C_ACK);
    send_data_i2c((uint8_t)(data >> 8), I2C_ACK);
    send_stop_i2c();
}


/*
Sets the direction of pin 'pin' on bank 'bank' to state 'dir'
pex - pointer to the pex device
bank - the bank the pin is on, A (GPIOA) or B (GPIOB)
pin - the pin that is having its direction set
dir - desired direction: OUTPUT or INPUT
*/
void set_pex_pin_dir(pex_t* pex, pex_bank_t bank, uint8_t pin, pex_dir_t dir) {
    uint8_t base = PEX_IODIR_BASE + bank;
    uint8_t register_state = read_pex_register(pex, base);
    switch (dir) {
        case OUTPUT:
            write_pex_register(pex, base, (register_state & ~_BV(pin)));
            break;
        case INPUT:
            write_pex_register(pex, base, (register_state | _BV(pin)));
            break;
    }
}

/*
Sets the value of pin 'pin' on bank 'bank' to value 'state'
pex - pointer to the pex device
pin - the pin to set
bank - the bank the pin is on, A (GPIOA) or B (GPIOB)
state - the value, must be 1 (HIGH) or 0 (LOW)
*/
void set_pex_pin(pex_t* pex, pex_bank_t bank, uint8_t pin, uint8_t state) {
    uint8_t base = PEX_GPIO_BASE + bank;
    uint8_t register_state = read_pex_register(pex, base);
    switch (state) {
        case 1:
            write_pex_register(pex, base, register_state | _BV(pin));
            break;
        case 0:
            write_pex_register(pex, base, register_state & ~_BV(pin));
            break;
    }
}

/*
Reads the state of `pin` on bank `bank` (either 0 or 1)
pex - pointer to the pex device
bank - the bank, A (GPIOA) or B (GPIOB)
pin - pin to read
*/
uint8_t get_pex_pin(pex_t* pex, pex_bank_t bank, uint8_t pin) {
    uint8_t base = PEX_GPIO_BASE + bank;
    uint8_t register_state = read_pex_register(pex, base);
    return (register_state >> pin) & 0b1;
}
