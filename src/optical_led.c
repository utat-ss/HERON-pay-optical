/*
DESCRIPTION: LED control functions for PAY-LED board
AUTHOR: Yong Da Li

Able to turn LEDs on PAY-LED boards on and off.
PAY-LED is 2 boards. Each board has 17 LED's and 1 PEX (port expander) 
PEX datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf

The PEX has 16 output channels. That means 2 of the LED will turn on together.
*there's actually 36 LED slots, but only 34 are populated --> 2 empty slots 

Accepts input format:
the physical location of the LED on the board, as uint8_t
    ex. 3rd from top left = 4
    ex. 1st on bottom row = 18

LED clock frequency = ~504 Hz (504 + 1/31 Hz)
PEX (port expander) with address 0b011 = 3 is on top edge of PAY-SSM
    - mounted with text upside down

PEX (port expander) with address 0b010 = 2 is on bottom edge of PAY-SSM
    - mounted with text rightside up
*/

#include "pwm.h"
#include "optical_led.h"

//preamble for PEX
pin_info_t cs = {
    .port = &PEX_CS_PORT_PAY_OPT,
    .ddr = &PEX_CS_DDR_PAY_OPT,
    .pin = PEX_CS_PIN_PAY_OPT
};

pin_info_t rst = {
    .port = &PEX_RST_PORT_PAY_OPT,
    .ddr = &PEX_RST_DDR_PAY_OPT,
    .pin = PEX_RST_PIN_PAY_OPT
};

pex_t top_pex = {
    .addr = TOP_PEX_ADDR,
    .cs = &cs,
    .rst = &rst
};

pex_t bot_pex = {
    .addr = BOT_PEX_ADDR,
    .cs = &cs,
    .rst = &rst
};

pexpin_t mf_channel_switcher[] = {
    //top left of pay-led board
    A2_4,
    A2_1,
    A2_2,
    A2_3,
    A2_6,
    A2_8,
    A2_5,
    A2_7,
    A3_1,

    //top right of pay-led board
    A3_4,
    A3_2,
    A3_3,
    A3_6,
    A3_8,
    A3_7,
    A3_5,
    A5_1,
    NO_CHANNEL,

    //bottom left of pay-led board
    A1_3,
    A1_2,
    A1_1,
    A1_4,
    A1_6,
    A1_8,
    A1_7,
    A1_5,
    A4_2,

    //bottom right of pay-led board
    A4_3,
    A4_1,
    A4_4,
    A4_6,
    A4_8,
    A4_7,
    A4_5,
    A5_2,
    NO_CHANNEL,
};

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
    // actually 504 + 1/31 Hz
    init_pwm_8bit(3, 30);

    init_pex(&top_pex); //PEX that goes on lower side of PAY-SSM 
    init_pex(&bot_pex); //PEX that goes on upper side of PAY-SSM (mounted with text upside-down)

    // PEX2 - set bank A to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&bot_pex, PEX_A, i, OUTPUT);

    // PEX2 - set bank B to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&bot_pex, PEX_B, i, OUTPUT);

    // PEX3 - set bank A to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&top_pex, PEX_A, i, OUTPUT);

    // PEX3 - set bank B to output
    for (uint8_t i = 0; i<8; i++)
        set_pex_pin_dir(&top_pex, PEX_B, i, OUTPUT);

    // turn all LED's off, total of 36 LEDs 
    // *actually only 34 LEDs and 2 empty spots
    for (uint8_t i = 0; i<36; i++){
        opt_led_board_position_off(i);
    }
}

// mode 0 = output
// mode 1 = input
void opt_led_set_mode_by_chn_pos(uint8_t channel_num, uint8_t mode){
    pexpin_t pexpin = mf_channel_switcher[channel_num];
    opt_led_set_mode(pexpin, mode);
}


void opt_led_set_mode(pexpin_t pexpin, uint8_t mode){
    uint8_t pex_addr = (pexpin & PEX_ADDR_MASK) >> 5; // leaves only pex_addr
    uint8_t pex_pin  = pexpin & PEXPIN_MASK; //leaves only pex_pin
    pex_t pex = top_pex;    // default to top board

    // (channels) 
    // pins 0-7 --> GPA, bank A
    // pins 8-15 --> GPB, bank B
    pex_bank_t bank = PEX_A; // default PEX_A

    if (pex_pin < 8){
        bank = PEX_A;
    }
    else
    {
        bank = PEX_B;
        pex_pin = pex_pin - 8;
    }

    //pex_addr 0b010 = 2 --> pex on bottom of pay-ssm
    //pex_addr 0b011 = 3 --> pex on top of pay-ssm
    switch(pex_addr){
        case (BOT_PEX_ADDR):
            pex = bot_pex;
            break;
        case (TOP_PEX_ADDR):
            pex = top_pex;
            break;
    }

    set_pex_pin_dir(&pex, bank, pex_pin, mode);
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
    uint8_t pex_addr = (pexpin & PEX_ADDR_MASK) >> 5;
    uint8_t pex_pin  = pexpin & PEXPIN_MASK;
    pex_t pex = top_pex;    // default to top board

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
    switch(pex_addr){
        case (BOT_PEX_ADDR):
            pex = bot_pex;
            break;
        case (TOP_PEX_ADDR):
            pex = top_pex;
            break;
    }

    //(address of pex, bank A or B, pex pin, 1 = high | 0 = low)
    set_pex_pin(&pex, bank, pex_pin, turn_on);
}


//turn on LED
void opt_led_on(pexpin_t pexpin){
    opt_led_switch(pexpin, 1);
}

//turn off LED
void opt_led_off(pexpin_t pexpin){
    opt_led_switch(pexpin, 0);
}


// accepts physical position of LED on board (0 - 33, inclusive)
// numbered left-to-right, top-down
// *includes the blank LED slot with no actual LED (right end of pay-led board)
void opt_led_board_position_on(uint8_t pos){
    pexpin_t pexpin = mf_channel_switcher[pos];
    opt_led_on(pexpin);
}

// accepts physical position of LED on board (0 - 33, inclusive)
// numbered left-to-right, top-down
// *includes the blank LED slot with no actual LED (right end of pay-led board)
void opt_led_board_position_off(uint8_t pos){
    pexpin_t pexpin = mf_channel_switcher[pos];
    opt_led_off(pexpin);
}


// print state of specified pex pin
void read_pex_pin(uint8_t pos){
    pexpin_t pexpin = mf_channel_switcher[pos];

    uint8_t pex_addr = (pexpin & PEX_ADDR_MASK) >> 5;
    uint8_t pex_pin  = pexpin & PEXPIN_MASK;
    pex_t pex = top_pex;    // default to top board

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
    switch(pex_addr){
        case (BOT_PEX_ADDR):
            pex = bot_pex;
            break;
        case (TOP_PEX_ADDR):
            pex = top_pex;
            break;
    }

    uint8_t state = get_pex_pin(&pex, bank, pex_pin);
    print("bank %d pin %d --> state %d\n", bank, pex_pin, state);
}


// print state of all pex pins
void read_all_pex_pins(void){
    for (uint8_t i = 0; i<36; i++)
        read_pex_pin(i);
}

// print state of all registers
void read_regs(void){
    uint8_t top_iodir_A = read_pex_register(&top_pex, PEX_IODIR_A);
    uint8_t top_iodir_B = read_pex_register(&top_pex, PEX_IODIR_B);
    uint8_t top_gpio_A = read_pex_register(&top_pex, PEX_GPIO_A);
    uint8_t top_gpio_B = read_pex_register(&top_pex, PEX_GPIO_B);

    uint8_t bot_iodir_A = read_pex_register(&bot_pex, PEX_IODIR_A);
    uint8_t bot_iodir_B = read_pex_register(&bot_pex, PEX_IODIR_B);
    uint8_t bot_gpio_A = read_pex_register(&bot_pex, PEX_GPIO_A);
    uint8_t bot_gpio_B = read_pex_register(&bot_pex, PEX_GPIO_B);

    print("\nRegisters:\n");

    print("top_iodir_A: 0x%.2X\n", top_iodir_A);
    print("top_iodir_B: 0x%.2X\n", top_iodir_B);
    print("top_gpio_A: 0x%.2X\n", top_gpio_A);
    print("top_gpio_B: 0x%.2X\n", top_gpio_B);
    print("bot_iodir_A: 0x%.2X\n", bot_iodir_A);
    print("bot_iodir_B: 0x%.2X\n", bot_iodir_B);
    print("bot_gpio_A: 0x%.2X\n", bot_gpio_A);
    print("bot_gpio_B: 0x%.2X\n", bot_gpio_B);
}