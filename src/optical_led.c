/*
DESCRIPTION: LED control functions for PAY-LED board
AUTHOR: Yong Da Li

Able to turn LEDs on PAY-LED boards on and off.
PAY-LED is 2 boards. Each board has 17 LED's and 1 PEX (port expander) 
PEX datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf

The PEX has 16 output channels. That means 2 of the LED will turn on together.
*there's actually 36 LED slots, but only 34 are populated --> 2 empty slots 

Accepts channel number (ex. A1_2) 
    or 
the physical location of the LED on the board.
ex. 3rd from top left = 4
ex. 1st on bottom row = 18


LED clock frequency = ~504 Hz (504 + 1/31 Hz)
PEX (port expander) with address 0b011 = 3 is on top edge of PAY-SSM
    - mounted with text upside down

PEX (port expander) with address 0b010 = 2 is on bottom edge of PAY-SSM
    - mounted with text rightside up
*/

#include "optical_led.h"

//only 1 PEX right now, need to implememt 2 PEX control
//preamble for PEX
pin_info_t cs = {
    .port = &PEX_CS_PORT_PAY,
    .ddr = &PEX_CS_DDR_PAY,
    .pin = PEX_CS_PIN_PAY
};

pin_info_t rst = {
    .port = &PEX_RST_PORT_PAY,
    .ddr = &PEX_RST_DDR_PAY,
    .pin = PEX_RST_PIN_PAY
};

pex_t pex = {
    .addr = PEX_ADDR_PAY,
    .cs = &cs,
    .rst = &rst
};

char[4] mf_channel = "A2_4" // defaults to LED on top-left of pay-led

/*
output direction for GPIO ports
    OUTPUT = 0
    INPUT = 1

bank A or bank B of GPIO ports
    PEX_A = 0
    PEX_B = 1
*/


void init_opt_led(void){
    /*
    init PWM 8-bit 

    Initializes and configures 8-bit timer for fast (single-slope) PWM output
    with variable frequence and fixed 50% duty cycle

    Prescaler: Divide the system clock (8 MHz) by one of five pre-set constants,
    (1, 8, 64, 256, 1024), 0 <= prescaler <= 4

    Top: Maximum value to which the counter will go before inverting the waveform,
    and resetting to 0x00, 0 <= top <= 255

    Frequency = 8/(prescaler * (top + 1) * 2) [Mhz]

    Duty-cyle: Invariably 50%, cannot be changed for this timer
    in the current configuration

    */

    // desired freq = 504 Hz
    // 0.000504 MHz = 8 / ( [256] * ([30]+1) *  2) 
    init_pwm_8bit(3, 30);

    init_uart();
    init_spi();
    init_pex(&pex2); //PEX that goes on lower side of PAY-SSM 
    init_pex(&pex3); //PEX that goes on upper side of PAY-SSM (mounted with text upside-down)

    // PEX2 - set bank A to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&pex2, PEX_A, i, OUTPUT);

    // PEX2 - set bank B to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&pex2, PEX_B, i, OUTPUT);

    // PEX3 - set bank A to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&pex3, PEX_A, i, OUTPUT);

    // PEX3 - set bank B to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&pex3, PEX_B, i, OUTPUT);

    // turn all LED's off, total of 36 LEDs 
    // *actually only 34 LEDs and 2 empty spots
    for (uint8_t i = 0; i<36; i++){
        opt_led_board_position_off(i);
    }
}

// enum maps channel number to PEX pin number
// top PEX has address: A(2), A(1), A(0) = 0b110 --> 3
// bottom PEX has address: A(2), A(1), A(0) = 0b010 --> 2
// enum bits encoding:
// 7    6    5    |  4    3    2    1    0
// __pex_address__|  ___5 bits for pin number(1-16)____ 

// pex_t* pex = actual PEX data type, contains cs and rst address info
// pexpin_t = info about which PEX (1 of 2) and which pin to turn on
// turn_on = 1 --> turns led on
// turn_on = 0 --> turns led off
void opt_led_switch(pexpin_t pexpin, uint8_t turn_on){
    uint8_t pex_addr = pexpin & PEX_ADDR_MASK;
    uint8_t pex_pin  = pexpin & PEXPIN_MASK;
    pex_t* pex = NULL;

    // (channels) 
    // pins 1-8 --> GPA, bank A
    // pins 9-16 --> GPB, bank B
    pex_bank_t bank = PEX_A;

    //pex_pin = 0 --> NO_CHANNEL, do nothing
    if (pex_pin == 0){
        return;
    }
    else if (pex_pin < 8){
        bank = PEX_A;
    }
    else
    {
        bank = PEX_B;
        pex_pin = pex_pin - 8;
    }

    //pex_addr 0b010 = 2 --> pex on bottom of pay-ssm
    //pex_addr 0b011 = 3 --> pex on top of pay-ssm
    switch(pex_addr >> 5){
        case (0b010):
            pex = pex2;
            break;
        case (0b011):
            pex = pex3;
            break;
    }

    //(address of pex, bank A or B, led pin, 1 = high | 0 = low)
    sex_pex_pin(&pex, bank, led, turn_on);
}


//turn on LED
void opt_led_on(pexpin_t pexpin){
    opt_led_switch(pexpin, 1);
}


// accepts string of microfluidics channel number, ex. A1_2
void opt_led_mf_on(char[] mf_channel){
    pexpin_t pexpin = opt_led_convert_mf(mf_channel);

    opt_led_on(pexpin);
}


//turn off LED
void opt_led_off(pexpin_t pexpin){
    opt_led_switch(pexpin, 0);
}


// accepts physical position of LED on board (0 - 33, inclusive)
// numbered left-to-right, top-down
// *includes the blank LED slot with no actual LED (right end of pay-led board)
void opt_led_board_position_on(uint8_t pos){
    char[] mf_channel = mf_channel_switcher[pos];
    pexpin_t pexpin = opt_led_convert_mf(mf_channel);

    opt_led_on(pexpin);
}



// accepts physical position of LED on board (0 - 33, inclusive)
// numbered left-to-right, top-down
// *includes the blank LED slot with no actual LED (right end of pay-led board)
void opt_led_board_position_off(uint8_t pos){
    mf_channel = mf_channel_switcher[pos];
    pexpin_t pexpin = opt_led_convert_mf(mf_channel);

    opt_led_off(pexpin);
}


// accepts string of microfluidics channel number, ex. A1_2
void opt_led_mf_off(char[] mf_channel){
    pexpin_t pexpin = opt_led_convert_mf(mf_channel);

    opt_led_off(pexpin);
}


// converts microfluidcs channel number (ex. A1_2) to pexpin_t
pexpin_t opt_led_convert_mf(char[] mf_channel){
    pexpin_t pexpin = A2_4; //default to LED on top left of board

    switch(mf_channel){
        //channel group 1
        case "A1_1":
            pexpin = A1_1;
            break;
        case "A1_2":
            pexpin = A1_2;
            break;
        case "A1_3":
            pexpin = A1_3;
            break;
        case "A1_4":
            pexpin = A1_4;
            break;
        case "A1_5":
            pexpin = A1_5;
            break;
        case "A1_6":
            pexpin = A1_6;
            break;
        case "A1_7":
            pexpin = A1_7;
            break;

        //channel group 2
        case "A2_1":
            pexpin = A2_1;
            break;
        case "A2_2":
            pexpin = A2_2;
            break;
        case "A2_3":
            pexpin = A2_3;
            break;
        case "A2_4":
            pexpin = A2_4;
            break;
        case "A2_5":
            pexpin = A2_5;
            break;
        case "A2_6":
            pexpin = A2_6;
            break;
        case "A2_7":
            pexpin = A2_7;
            break;

        //channel group 3
        case "A3_1":
            pexpin = A3_1;
            break;
        case "A3_2":
            pexpin = A3_2;
            break;
        case "A3_3":
            pexpin = A3_3;
            break;
        case "A3_4":
            pexpin = A3_4;
            break;
        case "A3_5":
            pexpin = A3_5;
            break;
        case "A3_6":
            pexpin = A3_6;
            break;
        case "A3_7":
            pexpin = A3_7;
            break;

        //channel group 4
        case "A3_1":
            pexpin = A3_1;
            break;
        case "A3_2":
            pexpin = A3_2;
            break;
        case "A3_3":
            pexpin = A3_3;
            break;
        case "A3_4":
            pexpin = A3_4;
            break;
        case "A3_5":
            pexpin = A3_5;
            break;
        case "A3_6":
            pexpin = A3_6;
            break;
        case "A3_7":
            pexpin = A3_7;
            break;
    }

    return pexpin;
}