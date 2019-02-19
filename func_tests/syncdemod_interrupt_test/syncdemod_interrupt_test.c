#include <uart/uart.h>
#include "../../src/syncdemod.h"

void interrupt0(void) {
    print("Interrupt 0\n");
}

void interrupt1(void) {
    print("Interrupt 1\n");
}

void interrupt2(void) {
    print("Interrupt 2\n");
}

void interrupt3(void) {
    print("Interrupt 3\n");
}

int main(void){
    init_uart();
    print("\n\n\nUART initialized\n");
    // syncdemod_init();
    syncdemod_init_interrupt(interrupt0, interrupt1, interrupt2, interrupt3);
    while(1) {}
}