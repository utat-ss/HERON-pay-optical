#include "main.h"

uint32_t spi_tx_data = 0;
bool spi_tx_data_in_progress = false;
uint8_t spi_tx_data_num_bytes_received = 0; // number of bytes already received by PAY


// Interrupts - see datasheet section 8
// SPI - see datasheet pg.133-...

// TODO - is this necessary? just interrupt?
void spi_slave_init(void) {
    /* Set MISO output, all others input */
    DDR_SPI = (1<<DD_MISO);
    /* Enable SPI */
    SPCR = (1<<SPE);
}

uint8_t spi_slave_receive(void) {
    /* Wait for reception complete */
    // TODO add timeout
    while(!(SPSR & (1<<SPIF)));
    /* Return data register */
    return SPDR;
}


/*
SPI Protocol with PAY:
When PAY sends a request to PAY-Optical to request reading of a sensor,
PAY will send a byte over SPI: {2'b11, field_number}
(2 bits of 1's, followed by 6 bits of the channel number)
*/

// TODO - after a SPI transfer finishes
ISR(SPI_STC_vect) {
    print("Interrupt - SPI serial transfer complete\n");
    uint8_t received = SPDR;
    print("Received byte: 0x%02x\n", received);
    SPDR = 0x00;

    if (spi_tx_data_in_progress) {
        spi_tx_data_num_bytes_received++;

        switch (spi_tx_data_num_bytes_received) {
            case 1:
                SPDR = (spi_tx_data >> 8) & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 2:
                SPDR = spi_tx_data & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 3:
                spi_tx_data = 0;
                spi_tx_data_in_progress = false;
                spi_tx_data_num_bytes_received = 0;
                break;
            default:
                print("Unexpected value of spi_tx_data_num_bytes_received: %u\n", spi_tx_data_num_bytes_received);
                break;
        }
    }

    else {
        // Check if the first 2 bits are 1's
        if (((received >> 6) & 0b11) == 0b11) {
            uint8_t field_number = received & 0x3F;
            read_sensor_command(field_number);
        }
    }
}


// field_number - a number from 0 to 32 (well number)
void read_sensor_command(uint8_t field_number) {
    uint8_t group = field_number / 8;
    uint8_t address = field_number % 8;

    // Enable the mux for the appropriate address
    // (this should turn on the LED and enable the amplifier)
    adc_optical_enable_mux(address + 1);

    // TODO - set up and configure synchronous demodulator

    uint8_t adc_channel;
    switch (group) {
        case 0:
            adc_channel = 5;
            break;
        case 1:
            adc_channel = 7;
            break;
        case 2:
            adc_channel = 9;
            break;
        case 3:
            adc_channel = 11;
            break;
        case 4:
            adc_channel = 13;
            break;
        default:
            print("Unexpected sensor group\n");
            adc_channel = 5;
            break;
    }

    // Read ADC data and prepare to send it over SPI
    spi_tx_data = adc_optical_read_raw_data(adc_channel, 1);
    adc_optical_disable_mux();
    SPDR = (spi_tx_data >> 16) & 0xFF;
    spi_tx_data_num_bytes_received = 0;
    spi_tx_data_in_progress = true;

    // Set DATA_RDY high
    set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
}


// TODO - place SPI data in SPDR beforehand
// maybe 1 for data ready, 0 for not ready?
int main(void) {
    init_uart();
    print("UART Initialized\n");

    adc_optical_init();
    print("ADC Initialized\n");

    spi_slave_init();
    print("SPI Initialized\n");

    init_cs(DATA_RDY_PIN, &DATA_RDY_DDR);
    set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
    print("DATA_RDY Initialized\n");
}
