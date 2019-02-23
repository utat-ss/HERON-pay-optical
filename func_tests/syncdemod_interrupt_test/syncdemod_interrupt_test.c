#include <uart/uart.h>
#include <util/atomic.h>
#include <utilities/utilities.h>
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"

uint8_t prescaler = 0;
uint16_t top = 0xF7;
uint16_t duty_cycle = 0x7B;

uint32_t volatile count[] = {0, 0, 0, 0};

void interrupt0(void) {
    // print("Interrupt 0\n");
    count[0] += 1;
}

void interrupt1(void) {
    //print("Interrupt 1\n");
    count[1] += 1;
}

void interrupt2(void) {
    // print("Interrupt 2\n");
    count[2] += 1;
}

void interrupt3(void) {
    // print("Interrupt 3\n");
    count[3] += 1;
}

int main(void){
    init_uart();
    print("\n\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_pwm_16bit(prescaler, top, duty_cycle);
    print("SENS CLK initialized to:\n");
    print("%.3f KHz\n", return_pwm_freq_16bit(prescaler, top));
    print("%.2f duty cyle\n\n", return_pwm_duty_16bit(top, duty_cycle));

    syncdemod_init();
    print("Synchronous demodulators started\n");

    syncdemod_init_interrupt(interrupt0, interrupt1, interrupt2, interrupt3);
    print("Interrupts Initialized\n");

    while(1) {

      for (int i = 0; i < 4; i++){
        // enable the synco drivers
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        syncdemod_write_synco(i+3, 1, SD_SYNCO_POLARITY_DEFAULT, SD_SYNCO_EDGE_DEFAULT);
        print("Enabled synco for channel: %d\n", i);
        }
        // wait some time
        _delay_ms(1000);
        // disable the synco drivers
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        syncdemod_write_synco(i+3, 0, SD_SYNCO_POLARITY_DEFAULT, SD_SYNCO_EDGE_DEFAULT);
        print("Count for channel %d: %d\n\n", i, count[i]);
        count[i] = 0;
        }
      }

    }
}
