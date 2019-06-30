#include "optical.h"

/* PORT EXPANDER AND MUX OBJECTS */
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

void all_on(void){
    set_pex_bank_pair(&OPT_PEX1, PEX_GPIO_A, 0xFFFF);
    set_pex_bank_pair(&OPT_PEX2, PEX_GPIO_A, 0xFFFF);
}

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
    print("-- Read 0x%04X from PEX\n", gpio_state);

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
    print("-- Writing 0x%04X to PEX\n", gpio_state);
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
