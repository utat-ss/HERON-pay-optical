/*
    PROTOCOL: to be written
    (see pay > src > optical.c for details)
*/

#include "spi_comms.h"

// Initialize SPI comms as SPI slave, with interrupts enabled
void init_spi_comms(void){
    // initialize SPI
    init_spi();

    // set DATA_RDYn pin as output high
    DATA_RDYn_DDR |= _BV(DATA_RDYn);    // set direction = output
    opt_set_data_rdy_high();
}


// return status of SS pin
// ------- might not need, since OPTICAL just checks SPIF flag
uint8_t opt_check_ss_pin(void){
    return SS_PIN >> SS; 
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
void opt_loop(void){
    // if SPI transfer if completed
    if (SPSR & _BV(SPIF)){
        // print("1\n");
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
        // print("2\n");
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

        // TODO
        // uint32_t reading = 0x123456;

        // spi_second_byte contains well_info
        opt_update_reading(spi_second_byte);    // performs reading (3 bytes), stores it in wells[32] of well_t
        
        // fetch reading from registers

        uint32_t reading = 0;  
        if ( ((spi_second_byte >> TEST_TYPE_BIT) & 0x1) == PAY_OPTICAL)    // bit 5 = 0
            reading = (wells + (spi_second_byte & 0x1F))->last_opt_reading;
        else // PAY_LED, bit 5 = 1
            reading = (wells + (spi_second_byte & 0x1F))->last_led_reading;
        
        opt_transfer_bytes(reading, NUM_GET_READING);       // shifts reading data into SPDR over 3 SPI transmissions
    }

    // get power
    else if (spi_first_byte == CMD_GET_POWER){
        uint32_t data = read_raw_power();
        opt_transfer_bytes(data, NUM_GET_POWER);
    }

    // invalid command
    else{ 
        uint8_t spi_status_byte = 0;

        spi_status_byte |= _BV(SPI_ERROR_BIT) | _BV(SPI_INVALID_COMMAND_BIT);
        SPDR = spi_status_byte;

        // start tranmsiission
        opt_set_data_rdy_low();

        // wait until SPI transfer is complete
        uint16_t timeout = UINT16_MAX;
        while (!(SPSR & _BV(SPIF)) && timeout>0){
            timeout--;
        }
        // read the SPDR to clear SPIF flag
        SPDR = SPDR & 0xFF;

        // done transmission
        opt_set_data_rdy_high();
    }
}


// calibrate and take well readings
// well_data[5] - optical density = 0, fluorescent LED = 1
// well_data[4:0] - well number (0-31)
void opt_update_reading(uint8_t well_info){
    update_well_reading((well_info & 0x1F), (well_info >> TEST_TYPE_BIT) & 0x1);
}

// sends multiple bytes via SPI, by sequentially shifting
// MSB sent first
void opt_transfer_bytes(uint32_t data, uint8_t num_bytes){
    // TODO - size?
    uint8_t tx_bytes[3] = {0x00};
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t shift = (3 - 1 - i) * 8;
        tx_bytes[i] = (data >> shift) & 0xFF;
    }

    print("SPI TX: ");
    print_bytes(tx_bytes, 3);

    // ex. 1 byte to transfer, shift = 0
    // ex. 4 bytes to transfer, shift = 24 -> 16 -> 8 -> 0    

    for (uint8_t i = 0; i < num_bytes; i++) {
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
        uint8_t dummy_byte = SPDR;
    }

    opt_set_data_rdy_high();
}


/*

Return the cmd corresponding to the opcode
Might be fast enough

cmd_t* cmd_opcode_to_cmd(uint8_t opcode){
    switch(opcode){
        case CMD_GET_READING:
            return &get_reading_cmd;
        case CMD_SET_LED_BANK:
            return &set_led_bank_cmd;
        case CMD_GET_LED_BANK:
            return &get_led_bank_cmd;
        case CMD_ENTER_SLEEP_MODE:
            return &enter_sleep_mode_cmd;
        case CMD_ENTER_NORMAL_MODE:
            return &enter_normal_mode_cmd;
        case CMD_GET_CURRENT:
            return &get_current_cmd;
        case CMD_GET_VOLTAGE:
            return &get_voltage_cmd;
        case CMD_GET_POWER:
            return &get_power_cmd;
        default:
            spi_status_byte |= _BV(SPI_ERROR_BIT) | _BV(SPI_INVALID_COMMAND_BIT);
            return &nop_cmd;
    }
}

// interrupt service routine runs when interrupt condition is tripped
ISR(SPI_STC_vect){
    // Evaluate the opcode
    spi_current_spdr_byte = SPDR;
    current_cmd = cmd_opcode_to_cmd(spi_current_spdr_byte);

    // *(current_cmd).fcn = current_cmd->fn = get function associated with command
    (current_cmd->fn)();
    SPDR = spi_status_byte;

    // while slave select is low
    while (!(SS_PIN & _BV(SS))){

        // while transmission has not ended
        // SPIF is asserted when 1 byte is shifted
        while (!(SPSR & _BV(SPIF)));

        spi_frame_number++;
        spi_current_spdr_byte = SPDR;
        (current_cmd->fn)();            // perform requested function

        // ATmega 328's SPI is double buffered in transmit direction
        // so while current data byte is being sent, start loading up 
        // the next byte to be sent using this buffer
        SPDR = spi_spdr_tx_buffer;
    }
    // reset control variables at end of ISR
    current_cmd = &nop_cmd;
    spi_frame_number = 0;
    spi_spdr_tx_buffer = 0;
}


// command functions
void nop_fn(void){
    // do nothing
}


void update_all_readings_fn(void){
    if (spi_frame_number == 1){
        
    }
}

// calibrate and take well readings
void update_reading_fn(void){
    if (spi_frame_number == 1){
        // update the well reading
        update_well_reading((spi_current_spdr_byte && 0x1F), spi_current_spdr_byte >> 7);
    }
}


// load well readings (which are already in data registers) for SPI transmission
void get_reading_fn(void){
    static uint32_t reading = 0;
    static pay_board_t board = 0;


    if (spi_frame_number == 1){
        board = spi_current_spdr_byte >> 7;
        if (board == PAY_OPTICAL){
            reading = (wells + (spi_current_spdr_byte && 0x1F))->last_opt_reading;
        } 
        else {
            reading = (wells + (spi_current_spdr_byte && 0x1F))->last_led_reading;
        }

        // load the next byte of data, ready for SPI transmission out
        spi_spdr_tx_buffer = (uint8_t)(reading >> 16);
    } 
    else if (spi_frame_number == 2){
        spi_spdr_tx_buffer = (uint8_t)(reading >> 8);
    } 
    else if (spi_frame_number == 3){
        spi_spdr_tx_buffer = (uint8_t)(reading);
    } 
    else {
        spi_spdr_tx_buffer = 0;
    }
}

void set_led_bank_fn(void);
void get_led_bank_fn(void);
void enter_sleep_mode_fn(void);
void enter_normal_mode_fn(void);

// 
void get_current_fn(void){
    static union {
        float val;
        uint8_t bytes[4];
    } current;

    if (spi_frame_number == 0) {
        current.val = power_read_current();
    } else if (spi_frame_number > 4) {
        spi_spdr_tx_buffer = 0;
    } else {
        // MSB first?
        spi_spdr_tx_buffer = current.bytes[4 - spi_frame_number];
    }
}

void get_voltage_fn(void) {
    static union {
        float val;
        uint8_t bytes[4];
    } voltage;

    if (spi_frame_number == 0) {
        voltage.val = power_read_voltage();
    } else if (spi_frame_number > 4) {
        spi_spdr_tx_buffer = 0;
    } else {
        // MSB first?
        spi_spdr_tx_buffer = voltage.bytes[4 - spi_frame_number];
    }
}


// very clever Vogel, abusing the union data structure
void get_power_fn(void) {
    static union {
        float val;
        uint8_t bytes[4];
    } power;

    if (spi_frame_number == 0) {
        power.val = power_read_power();
    } else if (spi_frame_number > 4) {
        spi_spdr_tx_buffer = 0;
    } else {
        // MSB first?
        spi_spdr_tx_buffer = power.bytes[4 - spi_frame_number];
    }
}

*/