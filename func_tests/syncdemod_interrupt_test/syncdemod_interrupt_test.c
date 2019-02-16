#include <uart/uart.h>
#include "../../src/syncdemod.h"



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
    syncdemod_init_interrupt(interrupt0, interrupt1, interrupt2, interrupt3);
    while(1) {}
}
