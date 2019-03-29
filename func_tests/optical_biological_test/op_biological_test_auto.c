#include <uart/uart.h>
#include <utilities/utilities.h>
#include <conversions/conversions.h>
#include "../../src/optical_adc.h"
#include "../../src/syncdemod.h"
#include "../../src/pwm.h"

#define CH5_EN_PORT		PORTC
#define CH5_EN_DDR		DDRC
#define CH5_EN			  PIN2

void read_values(uint8_t well_num, uint8_t channel_num);
void sweep_thru_channels(uint8_t);
void print_columns(void);


// A2 pairs up with A4
uint8_t A2_channels[] = {4, 1, 2, 3, 6, 8, 5, 7};
uint8_t A4_channels[] = {2, 3, 1, 4, 6, 8, 7, 5};
// A1 pairs up with A3
uint8_t A1_channels[] = {3, 2, 1, 4, 6, 8, 7, 5};
uint8_t A3_channels[] = {1, 4, 2, 3, 6, 8, 7, 5};

uint8_t *channels[] = {A1_channels, A2_channels, A3_channels, A4_channels};

void print_columns(void){
  print("\nWell,\tSensor,\tHex,\tPercent,\tVoltage,\n");
}

void read_values(uint8_t well_num, uint8_t channel_num){
  uint32_t data = opt_adc_read_sync();
  print("%d,\t%d,\t", well_num, channel_num);
  print("%06lX,\t%.6f,\t", data, (double) ((double) data / (double) 0xFFFFFF * 100.0));
  print("%.6f\n", -1.0 * opt_adc_raw_data_to_diff_vol(data, 1));
}

void sweep_thru_channels(uint8_t num){
  opt_adc_init_sync(num-1); // A1 channel
  syncdemod_enable_rclk(num + 2);
  //print("--Optical ADC A%d initialized\n\n", num);

  print_columns();
  for (uint8_t i = 0; i < 8; i++){
    uint8_t a = channels[num-1][i];
    // Initialize the differential pair corresponding to the bank number
    opt_adc_enable_mux(a - 1);
    read_values(i, a);
  }
  opt_adc_disable_mux();
}

// press "s" to start the automatic test
// enable gain once in the beginning
uint8_t rx_command(const uint8_t* buf, uint8_t len){
  static uint8_t echo = 0;
  static uint8_t gains[] = {1, 8, 16, 32, 64, 128};
  uint8_t gain;
  uint8_t recieved;

  if (echo){
    for (uint8_t i = 0; i < len; i++) {
        put_uart_char(buf[i]);
        if (buf[i] == 13){
          // recieved CR character
          echo = 0;
          print("\n--End of text\n");
        }
    }
  } if (!echo) {
    // convert from ASCII to decimal number
    recieved = buf[0] - 48;
    if (recieved == 0){
      //sweep the channels on bank A1
      sweep_thru_channels(1);
    } else if (recieved <= 6){
      gain = gains[recieved - 1];
      opt_adc_select_pga(gain);
      print("--Gain set: %d\n", gain);
    } else if (recieved == 7){
      print("--Start of text\n");
      echo = 1;
    } else if ((buf[0] == 10) | (buf[0] == 13)){
      // carriage return or line feed character
    } else {
      print("--Invalid command: %02X\n", buf[0]);
    }
  }
  return len;
}



int main(void){
  init_uart();
  set_uart_rx_cb(rx_command);
  print("--UART initialized\n");

  init_spi();
  print("--SPI initialized\n");

  init_output_pin(CH5_EN, &CH5_EN_DDR, 1);
  print("--Disabled Channel 5\n");

  // Should give 32.258 KHz, divided down by 64 on the syncdemod
  // About 504.032 Hz at the output, 50% duty
  init_pwm_16bit (0, 0xF7, 0x7B);
  print("--16-bit PWM initialized\n");

  opt_adc_init();
  // initilize the ADC
  print("--ADC Initialized\n");

  // initialize the syncdemod
  syncdemod_init();

  while(1){}
}
