#include <uart/uart.h>
#include <utilities/utilities.h>
#include <conversions/conversions.h>
#include "../../src/optical_adc.h"
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"

#define CH5_EN_PORT		PORTC
#define CH5_EN_DDR		DDRC
#define CH5_EN			  PIN2

void read_values(void);
void experiment(void);
void sweep_thru_channels(uint8_t);

// A2 pairs up with A4
uint8_t A2_channels[] = {4, 1, 2, 3, 6, 8, 5, 7};
uint8_t A4_channels[] = {2, 3, 1, 4, 6, 8, 7, 5};
// A1 pairs up with A3
uint8_t A1_channels[] = {3, 2, 1, 4, 6, 8, 7, 5};
uint8_t A3_channels[] = {1, 4, 2, 3, 6, 8, 7, 5};

uint8_t *channels[] = {A1_channels, A2_channels, A3_channels, A4_channels};

void read_values(void){
  uint32_t data = opt_adc_read_sync();

  print("Data = %06lX = %d %%\n", data, (int16_t) ((double) data / (double) 0xFFFFFF * 100.0));
  print("Voltage = %.6f V \n\n", -1 * opt_adc_raw_data_to_diff_vol(data, 1));
}

void sweep_thru_channels(uint8_t num){
  opt_adc_init_sync(num-1); // A4 channel
  print("Optical ADC A%d initialized\n", num);

  switch(num){
    case 1:
      syncdemod_enable_rclk(SD1_CS_PIN);
      break;
    case 2:
      syncdemod_enable_rclk(SD2_CS_PIN);
      break;
    case 3:
      syncdemod_enable_rclk(SD3_CS_PIN);
      break;
    case 4:
      syncdemod_enable_rclk(SD4_CS_PIN);
      break;
    default:
      print("number not valid");
      break;
  }

  for (uint8_t i = 0; i < 8; i++){
    uint8_t a = channels[num-1][i];
    opt_adc_enable_mux(a - 1);
    print("--Selected channel %d on A%d\n", a, num);
    read_values();
    _delay_ms(500);

    //resting for 3 seconds (with trash channel)
    opt_adc_enable_mux(0);
    for(uint8_t count = 3; count > 0; count--){
      _delay_ms(1000);
    }
  }
}

// press "s" to start the automatic test
// enable gain once in the beginning
uint8_t rx_command(const uint8_t* buf, uint8_t len){
  if (buf[0] == 0){
    // Perform a differential read
    print("Start Experiement");
    experiment();
  } else if ((buf[0] == 1) | (buf[0] == 2) | (buf[0] == 3) | (buf[0] == 4) | (buf[0] == 5) | (buf[0] == 6)){
    uint8_t gain;
    print("Gain Selection:\ninput  gain\n1  1\n2  8\n3  16\n4  32\n5  64\n6  128\n");
    switch(buf[0]){
      case 1:
        gain = 1;
        break;
      case 2:
        gain = 8;
        break;
      case 3:
        gain = 16;
        break;
      case 4:
        gain = 32;
        break;
      case 5:
        gain = 64;
        break;
      case 6:
        gain = 128;
        break;
      default:
        gain = 1;
        break;
    }
    opt_adc_select_pga(gain);
    print("--Gain set: %d\n", gain);
  } else {
    print("--Invalid command: %02X\n", buf[0]);
  }
  return len;
}

// automatic test starts
// diagonal section light up after one another
// delay each LED lighting up for 2 mins
void experiment(void){
  init_spi();
  print("SPI initialized\n");

  init_output_pin(CH5_EN, &CH5_EN_DDR, 1);
  print("Disabled Channel 5\n");

  // Should give 32.258 KHz, divided down by 64 on the syncdemod
  // About 504.032 Hz at the output, 50% duty
  init_pwm_16bit (0, 0xF7, 0x7B);
  print("16-bit PWM initialized\n");

  opt_adc_init();

  syncdemod_init();

  sweep_thru_channels(3);
}

int main(void){

  init_uart();
  set_uart_rx_cb(rx_command);
  print("\n\nUART initialized\n");

  //experiment();

  while(1){}

}
