/*
    Library to interface with the TSL2591

    Written by: Dylan Vogel
 */

#include "light_sens.h"

/*
Enable the TSL2591 and set default gain and integration time
*/
void init_light_sensor(light_sensor_t* light_sens){
    write_light_sense_register(LSENSE_ENABLE, LSENSE_DEF_ENABLE);
    write_light_sense_register(LSENSE_CONTROL, LSENSE_DEF_CONTROL);
    // Indicate that the sensor is enabled
    light_sens->state = LS_ENABLED;
    // Set the default values
    light_sens->gain = LS_LOW_GAIN;
    light_sens->time = LS_200ms;
}

/*
Put the TSL2591 to sleep
Disables the ALS and internal oscillator
*/
void sleep_light_sensor(light_sensor_t* light_sens){
    write_light_sense_register(LSENSE_ENABLE, 0x00);
    light_sens->state = LS_DISABLED;
}

/*
Wake up the TSL2591
Enables the ALS and the internal oscillator
Should be used instead of "init_light_sens" if you wish to save any
gain or intergation time settings previously written
*/
void wake_light_sensor(light_sensor_t* light_sens){
    write_light_sense_register(LSENSE_ENABLE, LSENSE_DEF_ENABLE);
    light_sens->state = LS_ENABLED;
}

/*
Write to one of the read/write registers on the TSL2591
addr: 4-bit register address
data: One byte of data to write
*/
void write_light_sense_register(uint8_t addr, uint8_t data){
    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_WRITE);
    send_data_i2c((LSENSE_COMMAND_BYTE | (addr & 0x1F)), I2C_ACK);
    send_data_i2c(data, I2C_ACK);
    send_stop_i2c();
}

/*
Read from one of the read/write registers on the TSL2591
 */
uint8_t read_light_sense_register(uint8_t addr){
    uint8_t data;

    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_WRITE);
    send_data_i2c((LSENSE_COMMAND_BYTE | (addr & 0x1F)), I2C_ACK);
    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_READ);
    read_data_i2c(&data, I2C_NACK);
    send_stop_i2c();

    return data;
}

/*
Return the values in all the read-only registers of the device
Must supply a uint8_t* pointer to an array of length 7
Faster than getting individual channel readings
*/
void get_light_sense_read_only(uint8_t* data){
    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS_READ_ONLY, I2C_READ);
    for (uint8_t i = 0; i < 6; i++){
        read_data_i2c((data + i), I2C_ACK);
    }
    read_data_i2c((data + 6), I2C_NACK);
    send_stop_i2c();
}

/*
Get CH0 and CH1 sensor readings
To get a correct reading of CH1, CH0 must be read first
See: https://forums.adafruit.com/viewtopic.php?f=19&t=124176 for reference
CH1 returned in the top two bytes, CH0 returned in the lower two bytes
*/
void get_light_sensor_readings(light_sensor_t* light_sens){
    uint8_t reth = 0;
    uint8_t retl = 0;
    uint16_t ch0_reading = 0;
    uint16_t ch1_reading = 0;

    while (!(read_light_sense_register(LSENSE_STATUS) & 0x01));

    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_WRITE);
    send_data_i2c((LSENSE_COMMAND_BYTE | LSENSE_C0DATAL), I2C_ACK);
    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_READ);
    read_data_i2c(&retl, I2C_ACK);
    read_data_i2c(&reth, I2C_NACK);

    ch0_reading = (uint16_t)((reth << 8) | retl);

    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_WRITE);
    send_data_i2c((LSENSE_COMMAND_BYTE | LSENSE_C1DATAL), I2C_ACK);
    send_start_i2c();
    send_addr_i2c(LSENSE_ADDRESS, I2C_READ);
    read_data_i2c(&retl, I2C_ACK);
    read_data_i2c(&reth, I2C_NACK);
    send_stop_i2c();

    ch1_reading = (uint16_t)((reth << 8) | retl);

    light_sens->last_ch0_reading = ch0_reading;
    light_sens->last_ch1_reading = ch1_reading;
}

/*
Set the gain bits in the TSL2591 CONTROL register to 
the gain value stored in the light_sens object
*/
void set_light_sensor_again(light_sensor_t* light_sens){
    // read the old control register value
    uint8_t control_value = read_light_sense_register(LSENSE_CONTROL);
    // insert the new gain bits
    control_value = (control_value & LSENSE_AGAIN_MASK) | ((light_sens->gain << 4) & ~LSENSE_AGAIN_MASK);
    // write back the new register value
    write_light_sense_register(LSENSE_CONTROL, control_value);
}

/*
Set the integration time bits in the TSL2591 CONTROL register to
the time value stored in the light_sens object
*/
void set_light_sensor_atime(light_sensor_t* light_sens){
    // read the old control register value
    uint8_t control_value = read_light_sense_register(LSENSE_CONTROL);
    // insert the new integration time bits
    control_value = (control_value & LSENSE_ATIME_MASK) | (light_sens->time & ~LSENSE_ATIME_MASK);
    // write back the new register value
    write_light_sense_register(LSENSE_CONTROL, control_value);
}