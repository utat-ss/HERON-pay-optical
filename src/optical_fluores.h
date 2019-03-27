#ifndef OPTICAL_LED_H
#define OPTICAL_LED_H
#endif

#include <uart/uart.h>
#include <spi/spi.h>
#include <utilities/utilities.h>
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"


void init_opt_fluores();

void opt_fluores_switch(uint8_t num, uint8_t state);
void opt_fluores_on(uint8_t num);
void opt_fluores_off(uint8_t num);