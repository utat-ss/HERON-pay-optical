#include "power.h"

pin_info_t load_switch_en = {
    .port = &LOAD_SWITCH_PORT,
    .ddr =  &LOAD_SWITCH_DDR,
    .pin = LOAD_SWITCH_PIN
};


/*
Initialize the power module
*/
void init_power(){
    init_adc();
    init_output_pin(load_switch_en.pin, load_switch_en.ddr, 1);
}

/*
Execute all important initializations for the board upon power-up
*/
void init_board(){
    init_uart();
    print("-- UART initialized\n");
    init_power();
    print("-- Power module initialized\n");
    init_i2c();
    print("-- I2C initialized\n");
    init_spi();
    print("-- SPI Initialized\n");
    init_board_sensors();
    print("-- Board initialized\n");

    init_spi_comms();
    print("-- SPI Comms initialized\n");
}

/*
Power cycle the board sensors and initialize them
*/
void init_board_sensors(){
    // Ensure that the sensors have been reset
    print("-- Power cycling the sensor ICs\n");
    // disable_sensor_power();
    // enable_sensor_power();

    init_all_pex();     // TODO: change to all pex once LED implemented
    print("-- Port expanders initialized\n");
    init_all_mux();
    print("-- Mux's initialized\n");
    init_opt_sensors();
    print("-- Light sensors initialized\n");
}

/*
Disable the power supply to the sensors
*/
void disable_sensor_power(){
    set_pin_low(load_switch_en.pin, load_switch_en.port);
    // testing showed that it took around 350 ms for the board to discharge
    _delay_ms(350);
}

/*
Enable the power supply to the sensors
*/
void enable_sensor_power(){
    set_pin_high(load_switch_en.pin, load_switch_en.port);
    // testing showed that it took < 1 ms for the board to charge
    _delay_ms(1);
}

/*
Puts the optical board into sleep mode
TODO: implement sleep mode on the micro
*/
void enter_sleep_mode(){
    disable_sensor_power();
    // TODO: do something with the micro
}

/*
Takes the optical board out of sleep mode
*/
void enter_normal_mode(){
    enable_sensor_power();
    init_board_sensors();
    // do something with the micro
}

/*
Return the current consumption of the board in mA
Returns total current going into the board from the SSM header
*/
float power_read_current(){
    uint16_t raw_data = read_adc_channel(POWER_CURR_CHANNEL);
    float current = convert_adc_data_to_voltage(raw_data, ADC_DEF_VREF);

    return current;
}

/*
Return the voltage of the sensor rail in V
Voltage is measured after load switch, so if the switch is disabled it should
read ~0V
*/
float power_read_voltage(){
    uint16_t raw_data = read_adc_channel(POWER_VOLT_CHANNEL);
    float voltage = convert_adc_data_to_voltage(raw_data, ADC_DEF_VREF);

    return voltage;
}

/*
Return the power being used by the board
If the load switch is disabled, the returned power should be ~0W because
the voltage is measured after the load switch. Just take a current measurement
and multiply by 3V3 to get "sleep" power.
*/
float power_read_power(){
    float current = power_read_current();
    float voltage = power_read_voltage();

    // see: ohm's law
    float power = voltage * current;

    return power;
}

/*
Convert an ADC reading into a voltage
Must supply the reference voltage
*/
float convert_adc_data_to_voltage(uint16_t data, float vref){
    // vin = (ADC * Vref) / 1024
    // see page 262 for reference
    float conversion = ((float)(data) * vref)/(_BV(10));

    return conversion;
}

/*
Initialize the ADC with default vref and prescaler
*/
void init_adc(){
    set_adc_vref(ADC_DEF_VREF);
    set_adc_prescaler(ADC_DEF_PRESCALER);
}

/*
Read the selected adc channel
TODO: Consider using with ADC noise reduction mode
*/
uint16_t read_adc_channel(uint8_t channel){
    uint16_t adc_read = 0x0000;

    set_adc_channel(channel);
    // enable ADC, single conversion, clear any ADIF flag, keep prescaler bits
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIF) | (ADCSRA & ~ADC_PRESCALER_MASK);
    while(!(ADCSRA & _BV(ADIF)));
    adc_read |= (uint16_t)(ADCL & 0x00FF);
    adc_read |= (uint16_t)((ADCH << 8) & 0x0300);
    // disable ADC, clear ADIF flag, keep prescaler bits
    ADCSRA = _BV(ADIF) | (ADCSRA & ~ADC_PRESCALER_MASK);

    return adc_read;
}

/*
Set the ADC channel bits in ADMUX
Input channels range from 0-7
1110 gives the internal 1.3V bandgap
1111 gives GND
See page 264-265 for reference
*/
void set_adc_channel(uint8_t channel){
    // mask channel bits, set new channel
    ADMUX = (ADMUX & ADC_MUX_MASK) | (channel & ~ADC_MUX_MASK);
}

/*
Set the ADC VREF bits in ADMUX
00 - AREF external
01 - AVCC
11 - Internal 2.56V reference
See page 264 for reference
 */

void set_adc_vref(uint8_t vref){
    // mask vref bits, set new vref
    ADMUX = (ADMUX & ADC_VREF_MASK) | ((vref << REFS0) & ~ADC_VREF_MASK);
}

/*
Set the ADC prescaler bits in ADCSRA
Division factor is 2^prescaler
 */
void set_adc_prescaler(uint8_t prescaler){
    // mask prescaler bits, set new prescaler
    ADCSRA = (ADCSRA & ADC_PRESCALER_MASK) | (prescaler & ~ADC_PRESCALER_MASK);
}
