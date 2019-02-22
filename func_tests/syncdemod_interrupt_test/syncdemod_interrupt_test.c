#include <uart/uart.h>
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"

uint8_t prescaler = 0;
uint16_t top = 0xF7;
uint16_t duty_cycle = 0x7B;



void interrupt0(void) {
    print("Interrupt 0");
}

void interrupt1(void) {
    print("Interrupt 1");
}

void interrupt2(void) {
    print("Interrupt 2");
}

void interrupt3(void) {
    print("Interrupt 3");
}

int main(void){
    init_uart();
    print("\n\n\nUART initialized\n");
    // syncdemod_init();


    init_pwm_16bit(prescaler, top, duty_cycle);
    print("SENS CLK initialized to:\n");
    print("%.2f KHz\n", return_pwm_freq_16bit(prescaler, top));
    print("%.2f % duty\n", return_pwm_duty_16bit(top, duty_cycle));

    syncdemod_init();
    print("Synchronous demodulators started");

    syncdemod_init_interrupt(interrupt0, interrupt1, interrupt2, interrupt3);
    print("Interrupts Initialized\n");

    while(1) {
      for (int i = 0; i < 4; i++){
        // enable the synco drivers
        syncdemod_write_synco(i+3, 1, SD_SYNCO_POLARITY_DEFAULT, SD_SYNCO_EDGE_DEFAULT);
        print("Enabled synco for channel: %d\n", i);
        // wait some time
        _delay_ms(100);
        // disable the synco drivers
        syncdemod_write_synco(i+3, 0, SD_SYNCO_POLARITY_DEFAULT, SD_SYNCO_EDGE_DEFAULT);
      }
    }
}
