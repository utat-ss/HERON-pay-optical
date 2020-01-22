#include "spi_comms.h"

// SPI data from PAY-SSM
volatile uint8_t spi_first_byte = 0;
volatile uint8_t spi_second_byte = 0;

volatile uint8_t spi_spdr_tx_buffer = 0;    // holds data to be shifted into SPDR, for SPI transfer
volatile uint8_t spi_frame_number = 0;      // byte number of data trasmission

volatile uint8_t spi_status_byte = 0;


// Initialize SPI comms as SPI slave, with interrupts enabled
void init_spi_comms(void){
    // initialize SPI
    init_spi();

    // set DATA_RDYn pin as output high
    DATA_RDYn_DDR |= _BV(DATA_RDYn);    // set direction = output
    opt_set_data_rdy_high();
}


// return status of SS pin
uint8_t opt_check_ss_pin(void){
    return SS_PIN & _BV(SS); 
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
        spi_first_byte = SPDR;
        opt_set_data_rdy_low();         // set DATA_RDYn low to signal to PAY that it's ready for the second byte

        // wait until another SPI transfer is completed
        // --> aka wait until SPIF is no longer high 
        while (!(SPSR & _BV(SPIF)));

        opt_set_data_rdy_high();        // clear DATA_RDYn, so PAY doesn't freak out
        spi_second_byte = SPDR;

        // now, got both bytes
        // perform the requested command and send back data if necessary
        manage_cmd(spi_first_byte);
    }
    opt_set_data_rdy_high();
}


// depending on cmd_code, does appropriate requested function + return data (if needed)
void manage_cmd (uint8_t cmd_code){
    // if first byte is get_reading, then 2nd byte is well data
    if (cmd_code == CMD_GET_READING){
        // spi_second_byte contains well_data
        opt_update_reading(spi_second_byte);    // performs reading (3 bytes), stores it in wells[32] of well_t
        opt_transfer_reading();       // shifts reading data into SPDR over 3 SPI transmissions
    }

    // invalid command
    else{ 
        spi_status_byte |= _BV(SPI_ERROR_BIT) | _BV(SPI_INVALID_COMMAND_BIT);
        SPDR = spi_status_byte;
        opt_set_data_rdy_low();

        // wait until SPI transfer is complete
        while (!(SPSR & _BV(SPIF)));
        opt_set_data_rdy_high();
    }

}


// calibrate and take well readings
// well_data[7] - optical density = 0, fluorescent LED = 1
// well_data[5:0] - well number (0-31)
void opt_update_reading(uint8_t well_data){
    update_well_reading((well_data && 0x1F), well_data >> 7);
}


void opt_transfer_reading(){
    uint32_t reading = 0;
    if (spi_second_byte >> 7 == PAY_OPTICAL)    // bit 7 = 0
        reading = (wells + (spi_second_byte && 0x1F))->last_opt_reading;
    else // PAY_LED, bit 7 = 1
        reading = (wells + (spi_second_byte && 0x1F))->last_led_reading;

    uint8_t shift = 16;
    while (shift >= 0){
        // load the next byte of data, ready for SPI transmission out
        spi_spdr_tx_buffer = (uint8_t)(reading >> shift);
        opt_set_data_rdy_low();     // signal to PAY to initiate SPI transfer

        // wait until SPI transfer is complete
        while (!(SPSR & _BV(SPIF)));
        opt_set_data_rdy_high();

        shift = shift - 8;

        // might need small delay here, will test
    }
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

cmd_t nop_cmd = {
    .fn = nop_fn,
    .opcode = 0xFF
};

cmd_t update_all_readings_cmd = {
    .fn = update_all_readings_fn,
    .opcode = CMD_UPDATE_ALL_READINGS
};

cmd_t update_reading_cmd = {
    .fn = update_reading_fn,
    .opcode = CMD_UPDATE_READING
};

cmd_t get_reading_cmd = {
    .fn = get_reading_fn,
    .opcode = CMD_GET_READING
};

cmd_t set_led_bank_cmd = {
    .fn = set_led_bank_fn,
    .opcode = CMD_SET_LED_BANK
};

cmd_t get_led_bank_cmd = {
    .fn = get_led_bank_fn,
    .opcode = CMD_GET_LED_BANK
};

cmd_t enter_sleep_mode_cmd = {
    .fn = enter_sleep_mode_fn,
    .opcode = CMD_ENTER_SLEEP_MODE
};

cmd_t enter_normal_mode_cmd = {
    .fn = enter_normal_mode_fn,
    .opcode = CMD_ENTER_NORMAL_MODE
};

cmd_t get_current_cmd = {
    .fn = get_current_fn,
    .opcode = CMD_GET_CURRENT
};

cmd_t get_voltage_cmd = {
    .fn = get_voltage_fn,
    .opcode = CMD_GET_VOLTAGE
};

cmd_t get_power_cmd = {
    .fn = get_power_fn,
    .opcode = CMD_GET_POWER
};



// LIST OF COMMANDS
cmd_t* all_cmds_list[] = {
    &update_all_readings_cmd,
    &update_reading_cmd,
    &get_reading_cmd,
    &set_led_bank_cmd,
    &get_led_bank_cmd,
    &enter_sleep_mode_cmd,
    &enter_normal_mode_cmd,
    &get_current_cmd,
    &get_voltage_cmd,
    &get_power_cmd
};

const uint8_t len_all_cmds = sizeof(all_cmds_list) / sizeof(all_cmds_list[0]);

*/