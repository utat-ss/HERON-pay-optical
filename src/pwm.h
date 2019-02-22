#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <util/atomic.h>
#include <utilities/utilities.h>

void init_pwm_8bit(uint8_t prescaler, uint8_t top);
void init_pwm_16bit(uint8_t prescaler, uint16_t top, uint16_t duty_cycle);
double return_pwm_freq_16bit(uint8_t prescaler, uint16_t top);
double return_pwm_duty_16bit(uint16_t top, uint16_t duty_cycle);
void stop_timer();
void start_timer();

#endif
