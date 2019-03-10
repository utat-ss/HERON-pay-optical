#include <uart/uart.h>
#include <utilities/utilities.h>
#include <conversions/conversions.h>
#include "../../src/optical_adc.h"
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"


#define CH5_EN_PORT		PORTC
#define CH5_EN_DDR		DDRC
#define CH5_EN			  PIN2

uint8_t rx_command(const uint8_t* buf, uint8_t len){
  if (buf[0] == 0){
    // Perform a differential read
    uint32_t data = opt_adc_read_sync();
    print("Data = %06lX = %d %%\n", data, (int16_t) ((double) data / (double) 0xFFFFFF * 100.0));
    print("Voltage = %.6f V \n\n", -1 * opt_adc_raw_data_to_diff_vol(data, 1));
  } else if (buf[0] <= 8){
    // Enable one of mux channel 0-8
    opt_adc_enable_mux(buf[0] - 1);
    print("--Selected channel %d\n", buf[0]);
  } else if ((buf[0] >> 4)  == 9){
    uint8_t gain;
    switch(buf[0] & 0xF){
      case 0:
        gain = 1;
        break;
      case 1:
        gain = 8;
        break;
      case 2:
        gain = 16;
        break;
      case 3:
        gain = 32;
        break;
      case 4:
        gain = 64;
        break;
      case 5:
        gain = 128;
        break;
      default:
        gain = 1;
        break;
    }
    opt_adc_select_pga(gain);
    print("--Gain set: %d\n", gain);
  } else if (buf[0] == 10){
    syncdemod_disable_rclk(SD2_CS_PIN);
    print("--Disabled RCLK\n");
  } else if (buf[0] == 11){
    syncdemod_enable_rclk(SD2_CS_PIN);
    print("--Enabled RCLK\n");
  } else {
    print("--Invalid command: %02X\n", buf[0]);
  }
  return len;
}


int main(void){

  init_uart();
  set_uart_rx_cb(rx_command);
  print("\n\nUART initialized\n");

  init_spi();
  print("SPI initialized\n");

  init_output_pin(CH5_EN, &CH5_EN_DDR, 1);
  print("Disabled Channel 5\n");

  // Should give 32.258 KHz, divided down by 64 on the syncdemod
  // About 504.032 Hz at the output, 50% duty
  init_pwm_16bit (0, 0xF7, 0x7B);
  print("16-bit PWM initialized\n");

  opt_adc_init();
  // Enable ADC differential channel for SD2
  opt_adc_init_sync(1);
  print("Optical ADC initialized\n");

  syncdemod_init();
  syncdemod_enable_rclk(SD1_CS_PIN);
  syncdemod_enable_rclk(SD2_CS_PIN);
  syncdemod_enable_rclk(SD3_CS_PIN);
  syncdemod_enable_rclk(SD4_CS_PIN);
  print("Enabled RCLK 2");

  print("\nStarting test\n\n");

  while (1) {
  }
}
