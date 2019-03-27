#include "optical_fluores.h"


// initializes blue fluorescent LED's
void init_opt_fluores(){
	init_spi();
	init_pwm_16bit (0, 0xF7, 0x7B);
	syncdemod_init();
}


// sets fluores LED in channel num either on or off, by state
void opt_fluores_switch(uint8_t num, uint8_t state){
	switch (state){
		case (1):
			syncdemod_enable_rclk(SD1_CS_PIN);
			break;
		case (0):
			syncdemod_disable_rclk(SD1_CS_PIN);
			break;
	}
}


// sets fluores LED in channel num on
void opt_fluores_on(uint8_t num){
	opt_fluores_switch(num, 1);
}


// sets fluores LED in channel num on
void opt_fluores_off(uint8_t num){
	opt_fluores_switch(num, 0);
}