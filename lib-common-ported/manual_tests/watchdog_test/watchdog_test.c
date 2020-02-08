#include <uart/uart.h>
#include <watchdog/watchdog.h>

// CPU clock frequency of 8 MHz
// Needs to be defined for using delay functions in the <util/delay.h> library
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>

//simulates a timeout

//set up watchdog timer

int main(void)
{
  init_uart();
  WDT_OFF();
  print("STARTING PROGRAM\n");

  WDT_ENABLE_SYS_RESET(WDTO_2S);

  for(int i = 0; i<5; i++)
  {
    _delay_ms(1500);
    WDT_ENABLE_SYS_RESET(WDTO_2S);
    print("LOOP%d\n",i);
  }

  int counter = 0;

  // set up an infinite loop
  // Should print around 20 times
  while(1){
    counter++;
    print("%d\n",counter);
    _delay_ms(100);
  }

  print("FAILED\n");
}
