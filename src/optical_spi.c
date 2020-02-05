/*
    PROTOCOL: to be written
    (see pay > src > optical.c for details)
*/

#include "optical_spi.h"

// Initialize SPI comms as SPI slave, with interrupts enabled
void init_opt_spi(void){
    // initialize SPI
    init_spi();

    // set DATA_RDYn pin as output high
    DATA_RDYn_DDR |= _BV(DATA_RDYn);    // set direction = output
    opt_set_data_rdy_high();
}

// set DATA_RDYn low
void opt_set_data_rdy_low(){
    DATA_RDYn_PORT &= ~_BV(DATA_RDYn);     // set output LOW
}

// set DATA_RDYn high
void opt_set_data_rdy_high(){
    DATA_RDYn_PORT |= _BV(DATA_RDYn);  // set output HIGH
}


// to be put in infinite loop in main
void opt_loop_main(void){
    // if SPI transfer if completed
    if (SPSR & _BV(SPIF)){
        // SPI data from PAY-SSM
        uint8_t rx_bytes[2] = {0x00};
        rx_bytes[0] = SPDR;

        // wait until another SPI transfer is completed
        // --> aka wait until SPIF is no longer high 
        uint32_t timeout = 1000;
        while (!(SPSR & _BV(SPIF)) && timeout>0){
            timeout--;
            _delay_ms(1);
        }
        if (timeout == 0) {
            print("TIMEOUT RX second byte\n");
        }
        rx_bytes[1] = SPDR;

        print("SPI RX: ");
        print_bytes(rx_bytes, 2);

        // now, got both bytes
        // perform the requested command and send back data if necessary
        manage_cmd(rx_bytes[0], rx_bytes[1]);
    }
    opt_set_data_rdy_high();
}


// depending on cmd_code, does appropriate requested function + return data (if needed)
void manage_cmd (uint8_t spi_first_byte, uint8_t spi_second_byte){
    // if first byte is get_reading, then 2nd byte is well info
    if (spi_first_byte == CMD_GET_READING){
        print("Get reading\n");

        // spi_second_byte contains well_info
        opt_update_reading(spi_second_byte);    // performs reading (3 bytes), stores it in wells[32] of well_t
        
        // fetch reading from registers

        uint32_t reading = 0;  
        if ( ((spi_second_byte >> OPT_TYPE_BIT) & 0x1) == PAY_OPTICAL)    // bit 5 = 1
            reading = (wells + (spi_second_byte & 0x1F))->last_opt_reading;
        else // PAY_LED, bit 5 = 1
            reading = (wells + (spi_second_byte & 0x1F))->last_led_reading;
        
        opt_transfer_bytes(reading);       // shifts reading data into SPDR over 3 SPI transmissions
    }

    // get power
    else if (spi_first_byte == CMD_GET_POWER){
        print("Get power\n");
        uint32_t data = read_raw_power();
        opt_transfer_bytes(data);
    }

    else if (spi_first_byte == CMD_ENTER_SLEEP_MODE) {
        print("Sleep mode\n");
        enter_sleep_mode();
        opt_transfer_bytes(0);
    }

    else if (spi_first_byte == CMD_ENTER_NORMAL_MODE) {
        print("Normal mode\n");
        enter_normal_mode();
        opt_transfer_bytes(0);
    }

    // else invalid command
}


// calibrate and take well readings
// well_data[5] - optical density = 0, fluorescent LED = 1
// well_data[4:0] - well number (0-31)
void opt_update_reading(uint8_t well_info){
    update_well_reading((well_info & 0x1F), (well_info >> OPT_TYPE_BIT) & 0x1);
}

// sends multiple bytes via SPI, by sequentially shifting
// MSB sent first
void opt_transfer_bytes(uint32_t data){
    uint8_t tx_bytes[SPI_TX_COUNT] = {0x00};
    for (uint8_t i = 0; i < SPI_TX_COUNT; i++) {
        uint8_t shift = (SPI_TX_COUNT - 1 - i) * 8;
        tx_bytes[i] = (data >> shift) & 0xFF;
    }

    print("SPI TX: ");
    print_bytes(tx_bytes, SPI_TX_COUNT);

    for (uint8_t i = 0; i < SPI_TX_COUNT; i++) {
        // load the next byte of data, ready for SPI transmission out
        SPDR = tx_bytes[i];
        opt_set_data_rdy_low();     // signal to PAY to initiate SPI transfer

        // wait until SPI transfer is complete
        uint32_t timeout = 1000;
        while (!(SPSR & _BV(SPIF)) && timeout > 0){
            timeout--;
            _delay_ms(1);
        }
        if (timeout == 0) {
            print("TIMEOUT in opt_transfer_bytes\n");
        }
        // Must read SPDR to clear SPIF bit or else optical will think it has
        // received another SPI transfer
        uint8_t dummy_byte __attribute__((unused)); // Silence unused variable warning
        dummy_byte = SPDR;
    }

    opt_set_data_rdy_high();
}
