#include "spi_comms.h"

volatile uint8_t spi_spdr_tx_buffer = 0;
volatile uint8_t spi_current_spdr_byte = 0; // current data byte being sent
volatile uint8_t spi_frame_number = 0;      // byte number of data trasmission
volatile cmd_t* current_cmd = &nop_cmd;
volatile uint8_t cmd_finished = 0;
volatile uint8_t spi_status_byte = 0;

void update_all_readings_fn(void);
void update_reading_fn(void);
void get_reading_fn(void);
void set_led_bank_fn(void);
void get_led_bank_fn(void);
void enter_sleep_mode_fn(void);
void enter_normal_mode_fn(void);
void get_current_fn(void);
void get_voltage_fn(void);
void get_power_fn(void);

/*
Initialize SPI comms as SPI slave, with interrupts enabled
*/
void init_spi_comms(void){
    // initialize SPI
    init_spi();
}

/*
Return the cmd corresponding to the opcode
Might be fast enough
*/
cmd_t* cmd_opcode_to_cmd(uint8_t opcode){
    switch(opcode){
        case CMD_UPDATE_ALL_READINGS:
            return &update_all_readings_cmd;
        case CMD_UPDATE_READING:
            return &update_reading_cmd;
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


/* COMMAND FUNCTIONS */
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

/*
*/
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



/* LIST OF COMMANDS */
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

