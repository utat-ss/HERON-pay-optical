/*
Optical SPI

This microcontroller (PAY-Optical) functions as a SPI slave. The PAY microcontroller functions as the SPI master.

SPI Protocol with PAY:
When PAY sends a request to PAY-Optical to request reading of a sensor,
PAY will send a byte over SPI: {2'b11, field_number} (2 bits of 1's, followed by 6 bits of the channel number)
field_number - a number from 0 to 35 (well number)
This is followed by 3 transactions for the 3 bytes of data.

There is a total of 4 SPI communications that happen in an exchange:
1. SSM requests data from a specific channel on optical
2. Optical transfers back the first byte of data
3. Optical transfers back the second byte of data
4. Optical transfers back the third byte of data

The interrupt on optical gets triggered on each of these.
Optical sets the DATA_RDY (data ready) pin high between transfers 1 and 2 to tell SSM it has the data ready
(synchronization between optical and ssm).
*/

#include "optical_spi.h"

// For testing, set this to true to use dummy SPI data as an example instead of
// reading the optical ADC
bool opt_spi_use_dummy_data = false;

// true if we are currently in the progress of sending data over SPI to PAY
bool spi_in_progress = false;
// 24-bit optical data we are currently sending
uint32_t spi_data = 0;
// Number of bytes already received by PAY
uint8_t spi_num_bytes_done = 0;

// A2 pairs up with A4
uint8_t A2_channels[] = {4, 1, 2, 3, 6, 8, 5, 7};
uint8_t A4_channels[] = {2, 3, 1, 4, 6, 8, 7, 5};
// A1 pairs up with A3
uint8_t A1_channels[] = {3, 2, 1, 4, 6, 8, 7, 5};
uint8_t A3_channels[] = {1, 4, 2, 3, 6, 8, 7, 5};

uint8_t *channels[] = {A1_channels, A2_channels, A3_channels, A4_channels};

// Interrupts - see datasheet section 8
// SPI - see datasheet pg.133-...


/*
Initializes the microcontroller to function as a SPI slave.
DO NOT INITIALIZE UART - it interferes with the SPI slave functionality
*/
void opt_spi_init(void) {
    // Set MISO and DATA_READY output, all others input
    // Using generic pins, not CS pins
    init_cs(MISO_A_PIN, &MISO_A_DDR);
    set_cs_low(MISO_A_PIN, &MISO_A_PORT);
    init_cs(DATA_RDY_PIN, &DATA_RDY_DDR);
    set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);

    /*
     * Enable SPI, set SPI control register
     * SPE: SPI0 enable
     * SPIE: SPI0 interrupt enable
     * SPR1: set SCK frequency
     */
    SPCR = (1 << SPE) | (1 << SPIE) | (1 << SPR1);

    sei(); // enable global interrupts
    SPSR |= (1 << SPIF); // also enable SPI interrupts
    MCUCR |= (1 << SPIPS); // use alternate SPI lines
}


// This interrupt is triggered just after PAY pulls PAY-Optical's CS pin low and transfers a byte of data
ISR(SPI_STC_vect) {
    // Get the received data from PAY from the SPI data register
    uint8_t received = SPDR;
    // Set the outgoing data to 0 by default
    SPDR = 0x00;

    // If we are currently in the process of transmitting SPI data,
    // i.e. already received the request for data but haven't sent all the bytes yet
    if (spi_in_progress) {
        spi_num_bytes_done++;

        /*
        Check how many bytes we have sent so far.
        If 1 or 2, we're not done yet so load the next byte into the SPI data register.
            Also pulse the DATA_RDY pin (need to set low before setting high so PAY receives the interrupt).
        If 3, we're done so stop sending data.
        */
        switch (spi_num_bytes_done) {
            case 1:
            	set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
                SPDR = (spi_data >> 8) & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 2:
            	set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
                SPDR = spi_data & 0xFF;
                set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
                break;
            case 3:
            	set_cs_low(DATA_RDY_PIN, &DATA_RDY_PORT);
                spi_in_progress = false;
                spi_data = 0;
                spi_num_bytes_done = 0;
                break;
            default:
            	// do nothing, although this is unexpected
                break;
        }
    }

    // If we are not currently in the process of transmitting SPI data
    else {
        // Check if we received a request for data (the first 2 bits are 1's)
        if (((received >> 6) & 0b11) == 0b11) {
            uint8_t field_number = received & 0x3F;

            // Setup to start sending data over SPI
            spi_in_progress = true;
            spi_num_bytes_done = 0;

            // Get the 24 bits of optical ADC data to send
            if (opt_spi_use_dummy_data) {
                // This is for testing SPI comms
                // Encode the field number in the last byte for reference
                spi_data = 0xdb6d00 | field_number;
            } else {
                // set SPI transfer lines to MISO_A and MISO_B (set to 0)
                MCUCR &= ~(1<<SPIPS);

                // set optical to Master
                SPCR |= (1<<MSTR);

                // This is for getting actual optical ADC data
                opt_adc_init();
                opt_adc_init_sync(field_number/8);
                opt_adc_enable_mux(channels[field_number/8][field_number%8]);
                spi_data = opt_adc_read_sync();

                // set SPI transfer lines back to normal lines,
                MCUCR |= (1<<SPIPS);

                // set optical to Slave, so SSM can read
                SPCR &= ~(1<<MSTR);
            }

            // Load the first byte into the data register
            SPDR = (spi_data >> 16) & 0xFF;
            // Set DATA_RDY high
            set_cs_high(DATA_RDY_PIN, &DATA_RDY_PORT);
        }
    }
}
