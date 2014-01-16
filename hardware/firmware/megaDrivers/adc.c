#include "adc.h"

// These two commands shouldn't be used with sendCommand() because they
// require multiple bytes to be sent/read.
#define CMD_RDATA   (0x12)
#define CMD_RREG    (0x20)
#define CMD_WREG    (0x40)

static uint8_t sample_data_buffer[19];
static bool contRead = true;

void initADC() {
	// Configure IO port
	PORTB |= 1;
	DDRB |= 0b111;

	// Configure ISP port
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA);
	SPSR |= (1<<SPI2X);
}

void writeRegister(uint8_t address, uint8_t value) {
	PORTB &= ~1;
	SPDR = CMD_WREG | address;
	while (!(SPSR & (1<<SPIF)));
	SPDR = 0;
	while (!(SPSR & (1<<SPIF)));
	SPDR = value;	
	while (!(SPSR & (1<<SPIF)));
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	PORTB |= 1;
}

void writeRegisters(uint8_t address, uint8_t *data, uint8_t count) {
	register uint8_t* endPtr = data + count;
	PORTB &= ~1;
	SPDR = CMD_WREG | address;
	count--; // Pre-decrement count while we are waiting for the transmit to complete
	while (!(SPSR & (1<<SPIF)));
	_delay_loop_1(4);
	SPDR = count;
	do {
		while (!(SPSR & (1<<SPIF)));
	_delay_loop_1(2);
		SPDR = *(data++);
	} while (data < endPtr);
	while (!(SPSR & (1<<SPIF))); // Need to wait for last byte
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	PORTB |= 1;
}

uint8_t readRegister(uint8_t address) {
	PORTB &= ~1;
	SPDR = CMD_RREG | (address & 0x1F);
	while (!(SPSR & (1<<SPIF)));
	SPDR = 0;
	while (!(SPSR & (1<<SPIF)));
	SPDR = 0;
	while (!(SPSR & (1<<SPIF)));
	PORTB |= 1;
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	return SPDR;
}

void readRegisters(uint8_t address, uint8_t *data, uint8_t count) {
	register uint8_t* endPtr = data + count;
	PORTB &= ~1;
	SPDR = CMD_RREG | address;
	count--; // Pre-decrement count while we are waiting for the transmit to complete
	while (!(SPSR & (1<<SPIF)));
	_delay_loop_1(4);
	SPDR = count;
	while (!(SPSR & (1<<SPIF)));
	do {
		_delay_loop_1(2);
		SPDR = 0;
		data++;
		while (!(SPSR & (1<<SPIF)));
		*data = SPDR;
	} while (data < endPtr);
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	PORTB |= 1;
}

// Sends command. Only used CMD_xxx defines from header
void sendCommand(uint8_t command) {
	if (command == CMD_RDATAC)
		contRead = true;
	else if (command == CMD_SDATAC)
		contRead = false;

	PORTB &= ~(1); // Assert CS
	SPDR = command;
	while (!(SPSR & (1<<SPIF))); // Wait for shift to complete
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	PORTB |=  1; // Deassert CS
}

void readData(int16_t *data, uint8_t channelCount) {
	uint8_t *cur_pntr = sample_data_buffer;
	uint8_t *end_pntr = sample_data_buffer + 3 + (channelCount<<1);
	
	PORTB &= ~1;
	if (!contRead) { // If we are not in continuous read mode, then we need to send read command
		SPDR = CMD_RDATA;
		while (!(SPSR & (1<<SPIF)));
		_delay_loop_1(3);
	}
	do {
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
		*(cur_pntr++) = SPDR;
		_delay_loop_1(1);
	} while (cur_pntr < end_pntr);
	_delay_loop_2(8); // Need to wait 2us after last falling edge before bringing CS high
	PORTB |= 1;
	
	switch (channelCount) {
	case 8: data[7] = (((int16_t)sample_data_buffer[17])<<8) | (int16_t)(sample_data_buffer[18]);
	case 7: data[6] = (((int16_t)sample_data_buffer[15])<<8) | (int16_t)(sample_data_buffer[16]);
	case 6: data[5] = (((int16_t)sample_data_buffer[13])<<8) | (int16_t)(sample_data_buffer[14]);
	case 5: data[4] = (((int16_t)sample_data_buffer[11])<<8) | (int16_t)(sample_data_buffer[12]);
	case 4: data[3] = (((int16_t)sample_data_buffer[9])<<8) | (int16_t)(sample_data_buffer[10]);
	case 3: data[2] = (((int16_t)sample_data_buffer[7])<<8) | (int16_t)(sample_data_buffer[8]);
	case 2: data[1] = (((int16_t)sample_data_buffer[5])<<8) | (int16_t)(sample_data_buffer[6]);
	case 1: data[0] = (((int16_t)sample_data_buffer[3])<<8) | (int16_t)(sample_data_buffer[4]);
	}

	//for (uint8_t cnt = 0; cnt < channelCount; cnt++)
	//	data[cnt] = (((int16_t)sample_data_buffer[3+(cnt<<1)])<<8) | (int16_t)(sample_data_buffer[4+(cnt<<1)]);
}

