/*
DESCRIPTION: toggles LEDs on/off on both PAY-LED boards in sequence
            - if you have only 1 board plugged it, it'll toggle those LEDs
            - then nothing will seem to happen
            - but it's actually trying to toggle the LED's on the other board, that's not plugged in
AUTHOR: Yong Da Li
*/

#include "../../src/optical_led.h"
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <spi/spi.h>

#include <test/test.h>

#ifndef F_CPU
#define F_CPU 8000000UL //8 MHz clock
#endif


void print_dirs() {
    print("bottom PEX - GPA Directions: %.2x\n", read_pex_register(&bot_pex, PEX_IODIR_A));
    print("bottom PEX - GPB Directions: %.2x\n", read_pex_register(&bot_pex, PEX_IODIR_B));

    print("top PEX - GPA Directions: %.2x\n", read_pex_register(&top_pex, PEX_IODIR_A));
    print("top PEX - GPB Directions: %.2x\n", read_pex_register(&top_pex, PEX_IODIR_B));
}


void print_values() {
    print("bottom PEX - GPA Values: %.2x\n", read_pex_register(&bot_pex, PEX_GPIO_A));
    print("top PEX - GPB Values: %.2x\n", read_pex_register(&bot_pex, PEX_GPIO_B));

    print("top PEX - GPA Values: %.2x\n", read_pex_register(&top_pex, PEX_GPIO_A));
    print("top PEX - GPB Values: %.2x\n", read_pex_register(&top_pex, PEX_GPIO_B));
}


int main(void){
    // UART line and PWM line are the same
    // so either you have print statements (UART init) or you turn the LED"s on (PWM)
    // you can't have both
    // init_uart();
    // print("\n\nUART initialized\n");

    init_spi();
    // print("SPI Initialized\n");
    // print("\n");

    // print("Starting test\n");

    init_opt_led();
    // print("Optical PEX initialized\n");
    
    uint8_t i = 0;
    while(1){
        //high
        //print("\npin high: %d\n", i);
        opt_led_board_position_on(i);
        _delay_ms(100);

        //low
        //print("\noutput: %d\n", i);
        opt_led_board_position_off(i);
        _delay_ms(100);

        i++;
        if (i>35)
            i = 0;
    }
    
   return 0;
}

/* this is what the actual SPI communication looks like

    LED's on or off
        pull CS low (start communications)
        send --> write control byte
        send --> address of register
        send --> state you want (1 = high, 0 = low)
        pull CS high (end communications)

    reading registers
        pull CS low (start communications)
        send --> read control byte
        send --> address of register
        send --> blank 8 bits, data back is the register data
        pull CS high (end communications)

    while(1){
        set_cs_low(PD1, &PORTD);
        send_spi(PEX_WRITE_CONTROL_BYTE | (2 << 1) );
        send_spi(PEX_GPIO_A);
        send_spi(0xf0);
        set_cs_high(PD1, &PORTD);
        _delay_us(500);

        set_cs_low(PD1, &PORTD);
        send_spi(PEX_READ_CONTROL_BYTE | (2 << 1) );
        send_spi(PEX_GPIO_A);
        uint8_t data = send_spi(0x00);
        set_cs_high(PD1, &PORTD);

        print("data: 0x %.2X\n", data);
        _delay_ms(1000);
    }*/

//============================================

// test code to see that registers are writing
    /*
    pex_bank_t bank = PEX_A;
    pex_dir_t dir = OUTPUT;
    uint8_t state = 0;
    uint8_t reg = 0;
    uint8_t pin = 1;

    dir = OUTPUT;
    // print("\npin mode %d, bank A --> output\n", pin);
    set_pex_pin_dir(&bot_pex1, bank, pin, dir);
    reg = read_pex_register(&bot_pex1, PEX_IODIR_A);
    // print("0x %.2X\n", reg);

    
    while(1){
        print("\npin mode %d, bank A --> output: OFF\n", pin);
        set_pex_pin(&bot_pex1, bank, pin, 0);
        _delay_ms(5);
        state = get_pex_pin(&bot_pex1, bank, pin);
        print("0x %.2X\n", state);

        _delay_ms(500);

        print("\npin mode %d, bank A --> output: ON\n", pin);
        //(address of pex, bank A or B, pex pin, 1 = high | 0 = low)
        set_pex_pin(&bot_pex1, bank, pin, 1);
        _delay_ms(5);
        state = get_pex_pin(&bot_pex1, bank, pin);
        print("0x %.2X\n", state);

        _delay_ms(500);
    }*/