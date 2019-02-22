/*
DESCRIPTION: Mappings for LEDs on PAY-LED boards
AUTHOR: Yong Da LI

Three things need to be linked:
1. (microfluidics) channel number pay-ssm board, ex. A1_2
2. left-right, top-down numbering of LEDs on the whole board, ex. 3rd LED from top left
3. pin number of port expander (PEX) that actually turns on the LED

There are 2 PEX's, each with 16 channels of output (32 "switches").
There are combined 34 LED's on both pay-optical boards.
The "switch" (aka PEX pin) for the left-most and right-most channels are connected
i.e. turning on the channel for the left-most LED also turns on the right-most channel.
    - this is PEX pin 2

optical_led.c accepts 2 input formats:
1. (microfluidics) channel number, written on the Pay-led board
    string of channel number (ex "A1_2") maps to a PEX pin, using the struct enum

2. physical location of LED, ex. 3rd from left
    [Use left to right, top-down numbering. Starting index is 0]
    leftmost on top = 0
    3rd from left = 4
    left on bottom = 18
    2nd rightmost on bottom = 33

    i'th location in array contains the (microfluidics) channel number string

    *note that the rightmost LED's on the top and bottom do not correspond to a microfluidics channel
    *but they are still counted in this numbering
*/

#ifndef OPTICAL_ADC_H
#define OPTICAL_ADC_H
#endif

#include <pex/pex.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <stdint.h>

// 2 constants not actually used
// freq is set by init_pwm_8bit()
// duty cycle for 8-bit pwm is fixed at 50%
#define PWM_LED_CLK     504 // 504 Hz
#define DUTY_CYCLE      0.5 // 50% duty cycle

#define PEXPIN_MASK     0b00011111 //leaves only pex pin --> zeros first 3 bits
#define PEX_ADDR_MASK   0b11100000 //leaves only pex address --> zeros last 5 bits

void init_opt_led(void);
void opt_led_switch(pexpin_t pexpin, uint8_t turn_on);

void opt_led_on(pexpin_t pexpin);
void opt_led_off(pexpin_t pexpin);

void opt_led_mf_on(char[] mf_channel);
void opt_led_mf_off(char[] mf_channel);
void opt_led_board_position_on(uint8_t pos);
void opt_led_board_position_off(uint8_t pos);

pexpin_t opt_led_convert_mf(char[] mf_channel);

/*
ADC on pay-optical board has 16 input channels
will be set in differential mode --> reading = difference in voltage
pair_num    (AIN+)    (AIN-)
0            5        6
1            7        8
2            9        10
3            11        12        *AIN13 and AIN14 are not used
4            15        16
*/

// A2-4 = use pair_num A"2" + 1 --> pair_num 3 = diff chn 11 and 12 on ADC

// enum maps channel number to PEX pin number
// top PEX has address: A(2), A(1), A(0) = 0b110 --> 3
// bottom PEX has address: A(2), A(1), A(0) = 0b010 --> 2
// enum bits encoding:
// 7    6    5    |  4    3    2    1    0
// __pex_address__|  ___5 bits for pin number(1-16)____   
typedef enum {
    //channel number = PEX address | PEX pin number
    A1_1 = 0b010<<5 | 3,
    A1_2 = 0b010<<5 | 2,
    A1_3 = 0b010<<5 | 1,
    A1_4 = 0b010<<5 | 4,
    A1_5 = 0b010<<5 | 8,
    A1_6 = 0b010<<5 | 5,
    A1_7 = 0b010<<5 | 7,
    A1_8 = 0b010<<5 | 6,

    A2_1 = 0b011<<5 | 1,
    A2_2 = 0b011<<5 | 16,
    A2_3 = 0b011<<5 | 15,
    A2_4 = 0b011<<5 | 2,
    A2_5 = 0b011<<5 | 12,
    A2_6 = 0b011<<5 | 14,
    A2_7 = 0b011<<5 | 11,
    A2_8 = 0b011<<5 | 13,

    A3_1 = 0b011<<5 | 10,
    A3_2 = 0b011<<5 | 8,
    A3_3 = 0b011<<5 | 7,
    A3_4 = 0b011<<5 | 9,
    A3_5 = 0b011<<5 | 3,
    A3_6 = 0b011<<5 | 6,
    A3_7 = 0b011<<5 | 4,
    A3_8 = 0b011<<5 | 5,

    A4_1 = 0b010<<5 | 11,
    A4_2 = 0b010<<5 | 9,
    A4_3 = 0b010<<5 | 10,
    A4_4 = 0b010<<5 | 12,
    A4_5 = 0b010<<5 | 16,
    A4_6 = 0b010<<5 | 13,
    A4_7 = 0b010<<5 | 15,
    A4_8 = 0b010<<5 | 14,

    A5_1 = 0b011<<5 | 2,
    A5_2 = 0b010<<5 | 1,

    //no channel number is all 0
    NO_CHANNEL = 0,
} pexpin_t;


// 5th index contains channel number of the 5th microfluidics channel
// physically on the pay-led board
pexpin_t [] mf_channel_switcher = {
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
}