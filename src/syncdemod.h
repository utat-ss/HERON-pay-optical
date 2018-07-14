#define SD_CS_PORT  PORTC
#define SD_CS_DDR   DDRC
#define SD1_CS_PIN  PC3
#define SD2_CS_PIN  PC4
#define SD3_CS_PIN  PC5
#define SD4_CS_PIN  PC6

#define SD_SERIAL_CONFIG_ADDR       0x0000
#define SD_FILTER_STROBE_ADDR       0x0010
#define SD_FILTER_CONFIG_BASE_ADDR  0x0010
#define SD_ANALOG_CONFIG_ADDR       0x0028
#define SD_SYNC_CONTROL_ADDR        0x0029
#define SD_DEMOD_CONTROL_ADDR       0x002A
#define SD_CLCK_CONFIG_ADDR         0x002B
#define SD_DIGITAL_CONFIG_ADDR      0x002C
#define SD_CORE_RESET_ADDR          0x002D

// No reset, MSB first, addr increment, 3-wire SPI
#define SD_SERIAL_CONFIG_DEFAULT    0x00

void syncdemod_init(void);
void syncdemod_write_register(uint16_t addr, uint8_t data);
void syncdemod_enable_rclk(void);
void syncdemod_disable_rclk(void);
void syncdemod_reset(void);
