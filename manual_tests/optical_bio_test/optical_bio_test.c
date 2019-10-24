#include <avr/io.h>
#include <stdint.h>
#include <utilities/utilities.h>
#include <uart/uart.h>
#include <pex/pex.h>
#include <i2c/i2c.h>
#include "../../src/optical.h"
#include "../../src/power.h"

void get_channel_readings(){
	uint32_t data = 0;
	uint8_t gain = 0;
	uint8_t time = 0;
  for (uint8_t j = 0; j < 100; j++){
    for (uint8_t i = 0; i < 8; i++){
      data = get_opt_sensor_reading(i, PAY_OPTICAL);
      gain = (uint8_t)(data >> 24);
      time = (uint8_t)((data >> 16) & 0x00FF);
      print("Sensor,%2d, Gain:,%02X, Time:,%02X, Value:,%lu,\n", i, gain, time, (data & 0x0000FFFF));
    }
	  print("\n");
  }
}

uint8_t rx_command(const uint8_t* buf, uint8_t len){
  static uint8_t echo = 0;
  uint8_t recieved;

  if (echo){
    for (uint8_t i = 0; i < len; i++) {
        put_uart_char(buf[i]);
        if (buf[i] == 13){
          // recieved CR character
          echo = 0;
		  print("--\n");
        }
    }
  } if (!echo) {
    // convert from ASCII to decimal number
    recieved = buf[0] - 48;
    if (recieved == 0){
      //sweep the channels on bank A1
      get_channel_readings();
    } else if (recieved == 7){
      print("--\n");
      echo = 1;
    } else if ((buf[0] == 10) | (buf[0] == 13)){
      // carriage return or line feed character
    } else {
      print("--Invalid command: %02X\n", buf[0]);
    }
  }
  return len;
}

int main(void) {
	init_board();
	set_uart_rx_cb(rx_command);

	while (1);
}


