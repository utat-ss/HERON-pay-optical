/*
Optical SPI

This microcontroller functions as a SPI slave. The PAY microcontroller functions as the SPI master.
*/

#include "optical_spi.h"


bool spi_in_progress = false;
uint32_t spi_data = 0;
uint8_t spi_num_bytes_done = 0; // number of bytes already received by PAY


// Interrupts - see datasheet section 8
// SPI - see datasheet pg.133-...

void opt_spi_init(void) {
    /* Set MISO output, all others input */
    DDR_SPI = 1 << DD_MISO;
    /* Enable SPI */
    SPCR = 1 << SPE;
}


// TODO - is this necessary? just interrupt?
// TODO - does receiving happen automatically?
uint8_t opt_spi_receive(void) {
    /* Wait for reception complete */
    // TODO add timeout
    while (!(SPSR & (1 << SPIF)));
    // Set DATA_RDY low
    set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
    /* Return data register */
    return SPDR;
}




/*
SPI Protocol with PAY:
When PAY sends a request to PAY-Optical to request reading of a sensor,
PAY will send a byte over SPI: {2'b11, field_number}
(2 bits of 1's, followed by 6 bits of the channel number)
field_number - a number from 0 to 32 (well number)
*/
// TODO - after a SPI transfer finishes
ISR(SPI_STC_vect) {
    print("Interrupt - SPI serial transfer complete\n");
    uint8_t received = SPDR;
    print("Received byte: 0x%02x\n", received);
    SPDR = 0x00;

    if (spi_in_progress) {
        spi_num_bytes_done++;

        switch (spi_num_bytes_done) {
            case 1:
                SPDR = (spi_data >> 8) & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 2:
                SPDR = spi_data & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 3:
                spi_in_progress = false;
                spi_data = 0;
                spi_num_bytes_done = 0;
                break;
            default:
                print("Unexpected num_bytes_done: %u\n", spi_num_bytes_done);
                break;
        }
    }

    else {
        // Check if the first 2 bits are 1's
        if (((received >> 6) & 0b11) == 0b11) {
            uint8_t field_number = received & 0x3F;

            spi_in_progress = true;
            spi_data = opt_adc_read_field_raw_data(field_number);
            spi_num_bytes_done = 0;

            // Set the data register
            SPDR = (spi_data >> 16) & 0xFF;
            // Set DATA_RDY high
            set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
        }
    }
}
