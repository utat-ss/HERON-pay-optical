#include "optical.h"

/* PORT EXPANDER OBJECTS */
pex_t OPT_PEX1 = {
    .addr = OPTICAL_PEX1_ADDR,
    .rst = NULL
};

pex_t OPT_PEX2 = {
    .addr = OPTICAL_PEX2_ADDR,
    .rst = NULL
};

pex_t LED_PEX1 = {
    .addr = LED_PEX1_ADDR,
    .rst = NULL
};

pex_t LED_PEX2 = {
    .addr = LED_PEX2_ADDR,
    .rst = NULL
};

/* MUX OBJECTS */
pin_info_t I2C_MUX1_RST = {
    .port = &PORTC,
    .ddr = &DDRC,
    .pin = PC3
};

pin_info_t I2C_MUX2_RST = {
    .port = &PORTC,
    .ddr = &DDRC,
    .pin = PC2
};

pin_info_t I2C_MUX3_RST = {
    .port = &PORTC,
    .ddr = &DDRC,
    .pin = PC1
};

pin_info_t I2C_MUX4_RST = {
    .port = &PORTC,
    .ddr = &DDRC,
    .pin = PC0
};

mux_t OPT_MUX1 = {
    .addr = I2C_MUX1_ADDR,
    .rst = &I2C_MUX1_RST
};

mux_t OPT_MUX2 = {
    .addr = I2C_MUX2_ADDR,
    .rst = &I2C_MUX2_RST
};

mux_t OPT_MUX3 = {
    .addr = I2C_MUX3_ADDR,
    .rst = &I2C_MUX3_RST
};

mux_t OPT_MUX4 = {
    .addr = I2C_MUX4_ADDR,
    .rst = &I2C_MUX4_RST
};

/* OPTICAL SENSORS */

light_sensor_t opt_sensors[32];

/*
Initialize the global array of optical sensors
*/
void init_opt_sensors(void){
    for (uint8_t i = 0; i < 32; i++){
        init_light_sensor(opt_sensors + i);
    }
}

/*
Return the sensor reading for channel pos of type meas
bits[25:24] are gain
bits[18:16] are integration time
*/
uint32_t get_opt_sensor_reading(uint8_t pos, pay_board_t board){
    mux_t* mux = NULL;
    uint8_t channel = pos % 8;
    uint32_t ret = 0;

    switch (pos/8) {
        case 0:
            mux = &OPT_MUX1;
            break;
        case 1:
            mux = &OPT_MUX2;
            break;
        case 2:
            mux = &OPT_MUX3;
            break;
        case 3:
            mux = &OPT_MUX4;
            break;
        default:
            // ya goofed
            return ret;
    }

    set_led(pos, board, LED_ON);
    set_mux_channel(mux, channel);

    calibrate_opt_sensor_sensitivity(opt_sensors + pos); 

    disable_all_mux_channels(mux);      
    set_led(pos, board, LED_OFF);

    // TODO: do something smarter
    ret = opt_sensors[pos].last_ch0_reading | ((uint32_t)(opt_sensors[pos].time) << 16) | ((uint32_t)(opt_sensors[pos].gain) << 24);

    return ret;
}

/*
Take readings from the optical sensor and calibrate gain and integration time
to extract maximum dynamic range
*/
void calibrate_opt_sensor_sensitivity(light_sensor_t* light_sens){
    float last_reading = 0.0;
    uint8_t calibrated = 0;

    get_light_sensor_readings(light_sens);
    last_reading = (float)(light_sens->last_ch0_reading) / (float)(1UL << 16);

    if ((last_reading > OPT_SENS_LOW_THRES) && (last_reading < OPT_SENS_HIGH_THRES)){
        calibrated = 1;
    }

    while (!calibrated){
        // TODO: surely there is a more elegant way to do this
        get_light_sensor_readings(light_sens);
        last_reading = (float)(light_sens->last_ch0_reading) / (float)(1UL << 16);

        if (last_reading < OPT_SENS_LOW_THRES){
            if (light_sens->time != LS_600ms){
                light_sens->time += 1;                  // move to higher integration time
                set_light_sensor_atime(light_sens);     // write new value
                continue;
            } else if (light_sens->gain != LS_MAX_GAIN){
                light_sens->gain += 1;
                light_sens->time = LS_200ms;
                set_light_sensor_again(light_sens);     // move to higher gain range
                set_light_sensor_atime(light_sens);     // move to lowest integration time
                continue;
            } else {
                calibrated = 1;                      // nothing we can do, measurement undersaturated
                break;
            }
        } else if (last_reading > OPT_SENS_HIGH_THRES){
            if (light_sens->time != LS_200ms){
                light_sens->time -= 1;                  // move to lower integration time
                set_light_sensor_atime(light_sens);     // write value
                continue;
            } else if (light_sens->gain != LS_LOW_GAIN){
                light_sens->gain -= 1;                  
                light_sens->time = LS_600ms;            
                set_light_sensor_again(light_sens);     // move to lower gain value
                set_light_sensor_atime(light_sens);     // move to highest integration time
                continue;
            } else {
                calibrated = 1;
                break;                                  // nothing to do
            }
        } else {
            // yay we did it! sensor is calibrated
            calibrated = 1;
            break;
        }
    }
  
    // calling function should pull the last sensor value from light_sens
}

/*
Turn on all LEDs
*/
void all_on(void){
    set_pex_bank_pair(&OPT_PEX1, PEX_GPIO_A, 0xFFFF);
    set_pex_bank_pair(&OPT_PEX2, PEX_GPIO_A, 0xFFFF);
}

/*
Turn off all LEDs
*/
void all_off(void){
    set_pex_bank_pair(&OPT_PEX1, PEX_GPIO_A, 0x0000);
    set_pex_bank_pair(&OPT_PEX2, PEX_GPIO_A, 0x0000);
}

/*
Initialize ALL the port expanders
*/
void init_all_pex(void){
    init_opt_pex();
    init_led_pex();
}

/*
Initialize all port expanders on the optical board
*/
void init_opt_pex(void){
    init_pex(&OPT_PEX1);
    init_pex(&OPT_PEX2);
    // set all pex pins to low pre-emptively
    set_pex_bank_pair(&OPT_PEX1, PEX_GPIO_A, 0x0000);
    set_pex_bank_pair(&OPT_PEX2, PEX_GPIO_A, 0x0000);
    // set all pex pins to output
    set_pex_bank_pair(&OPT_PEX1, PEX_IODIR_A, 0x0000);
    set_pex_bank_pair(&OPT_PEX2, PEX_IODIR_A, 0x0000);
}

/*
Initialize all port expanders on the LED board
*/
void init_led_pex(void){
    init_pex(&LED_PEX1);
    init_pex(&LED_PEX2);
    // set all pex pins to low pre-emptively
    set_pex_bank_pair(&LED_PEX1, PEX_GPIO_A, 0x0000);
    set_pex_bank_pair(&LED_PEX2, PEX_GPIO_A, 0x0000);
    // set all pex pins to output
    set_pex_bank_pair(&LED_PEX1, PEX_IODIR_A, 0x0000);
    set_pex_bank_pair(&LED_PEX2, PEX_IODIR_A, 0x0000);
}

/*
Sets the LED at pos to the desired state
pos: uint8_t between 0 and 31
board: either PAY_OPTICAL or PAY_LED
state: either LED_ON (1) or LED_OFF (0)
*/
void set_led(uint8_t pos, pay_board_t board, led_state_t state){
    // fun fact, if your input is out of range you'll just do nothing

    pex_t* pex = NULL;

    // get the corresponding port expander
    switch (board){
        case PAY_OPTICAL:
            if (pos < 16) {
                pex = &OPT_PEX1;
            } else {
                pex = &OPT_PEX2;
                pos -= 16;
            }
            break;
        case PAY_LED:
            if(pos < 16) {
                pex = &LED_PEX1;
            } else {
                pex = &LED_PEX2;
                pos -= 16;
            }
            break;
    }

    // read the current GPIO state
    uint16_t gpio_state = get_pex_bank_pair(pex, PEX_GPIO_A);

    switch (state){
        case LED_ON:
            gpio_state |= _BV(pos);
            break;
        case LED_OFF:
            gpio_state &= ~(_BV(pos));
            break;
        default:
            // do nothing
            break;
    }

    // write back the desired LED state
    set_pex_bank_pair(pex, PEX_GPIO_A, gpio_state);
}


/*
Returns the state of the LED at pos
pos: uint8_t between 0 and 31
board: either PAY_OPTICAL or PAY_LED
*/
uint8_t get_led(uint8_t pos, pay_board_t board){
    pex_t* pex = NULL;

    // get the corresponding port expander
    switch (board){
        case PAY_OPTICAL:
            if (pos < 16) {
                pex = &OPT_PEX1;
            } else {
                pex = &OPT_PEX2;
                pos -= 16;
            }
            break;
        case PAY_LED:
            if(pos < 16) {
                pex = &LED_PEX1;
            } else {
                pex = &LED_PEX2;
                pos -= 16;
            }
            break;
    }

    // read the current GPIO state
    uint16_t gpio_state = get_pex_bank_pair(pex, PEX_GPIO_A);
    uint8_t state = (gpio_state >> pos) & 0x01;

    return state;
}
