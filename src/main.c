#include "main.h"

uint32_t spi_tx_data = 0;
bool spi_tx_data_in_progress = false;
uint8_t spi_tx_data_num_bytes_done = 0; // number of bytes already received by PAY


// Interrupts - see datasheet section 8
// SPI - see datasheet pg.133-...

void spi_slave_init(void) {
    /* Set MISO output, all others input */
    DDR_SPI = (1<<DD_MISO);
    /* Enable SPI */
    SPCR = (1<<SPE);
}


// TODO - is this necessary? just interrupt?
uint8_t spi_slave_receive(void) {
    /* Wait for reception complete */
    // TODO add timeout
    while(!(SPSR & (1<<SPIF)));
    /* Return data register */
    return SPDR;
}


// field_number - a number from 0 to 32 (well number)
void start_read_optical_command(uint8_t field_number) {
    spi_tx_data = opt_adc_read_field_raw_data(field_number);

    SPDR = (spi_tx_data >> 16) & 0xFF;
    spi_tx_data_num_bytes_done = 0;
    spi_tx_data_in_progress = true;

    // Set DATA_RDY high
    set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
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
        spi_tx_data_num_bytes_done++;

        switch (spi_tx_data_num_bytes_done) {
            case 1:
                SPDR = (spi_tx_data >> 8) & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 2:
                SPDR = spi_tx_data & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 3:
                spi_tx_data_in_progress = false;
                spi_tx_data = 0;
                spi_tx_data_num_bytes_done = 0;
                break;
            default:
                print("Unexpected num_bytes_done: %u\n", spi_tx_data_num_bytes_done);
                break;
        }
    }

    else {
        // Check if the first 2 bits are 1's
        if (((received >> 6) & 0b11) == 0b11) {
            uint8_t field_number = received & 0x3F;
            start_read_optical_command(field_number);
        }
    }
}


int main(void) {
    init_uart();
    print("UART Initialized\n");

    opt_adc_init();
    print("Optical ADC Initialized\n");

    spi_slave_init();
    print("SPI Slave Initialized\n");

    init_cs(DATA_RDY_PIN, &DATA_RDY_DDR);
    set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
    print("DATA_RDY Initialized\n");

    while (1) {}
}
