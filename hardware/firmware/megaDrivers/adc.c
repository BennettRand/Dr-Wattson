#include "adc.h"

// These two commands shouldn't be used with sendCommand() because they
// require multiple bytes to be sent/read.
#define CMD_RDATA   (0x12)
#define CMD_RREG    (0x20)
#define CMD_WREG    (0x40)

//------------ SPI Interrupt Communication Variables and ISR -----------------
static bool spi_busy;
static uint8_t *out_data_pntr;
static uint8_t out_data_count = 0;
static uint8_t *in_data_pntr;
static uint8_t in_data_count = 0;
static void (*complete_callback)(void);

static uint8_t current_data_byte = 0;

// SPI transmit complete interrupt
ISR(SPI_STC_vect) {
	current_data_byte++;

	if (in_data_count >= current_data_byte)
		in_data_pntr[current_data_byte-1] = SPDR;

	if (out_data_count > current_data_byte)
		SPDR = out_data_pntr[current_data_byte];
	else if (in_data_count > current_data_byte) // if all data has been shifted out but there is more to shift in.
		SPDR = 0;
	else {
		PORTB |= 1;
		_delay_us(1); // Enforce CS high pulse length
		spi_busy = false;
		if (complete_callback != 0)
			complete_callback();
	}
}
//----------------------------------------------------------------------------

static uint8_t regCommand[22]; // Defined this way so that regCommand and regValue are contiguous
static uint8_t *regValue = &(regCommand[2]);

static uint8_t sample_data_buffer[19];
struct adc_data_packet adc_data_sample;

void initADC() {
	// Configure IO port
	PORTB |= 1;
	DDRB |= 0b111;

	// Configure ISP port
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA);
}

void writeRegister(uint8_t address, uint8_t value) {
	while (spi_busy)
	spi_busy = true; // Take control of the port

	regCommand[0] = CMD_WREG | (address & 0x1F);
	regCommand[1] = 0;
	regValue[0] = value;
	out_data_pntr = regCommand;
	out_data_count = 3;
	in_data_count = 0;
	current_data_byte = 0;
	complete_callback = 0;

	PORTB &= 1;
	SPDR = regCommand[0];

	return;
}

void writeRegisters(uint8_t address, uint8_t *data, uint8_t count) {
	while (spi_busy)
	spi_busy = true; // Take control of the port

	regCommand[0] = CMD_WREG | (address & 0x1F);
	regCommand[1] = count - 1;
	memcpy(regValue, data, count);
	out_data_pntr = regCommand;
	out_data_count = 2 + count;
	in_data_count = 0;
	current_data_byte = 0;
	complete_callback = 0;

	PORTB &= 1;
	SPDR = regCommand[0];

	return;
}

void startReadRegister(uint8_t address) {
	while (spi_busy)
	spi_busy = true; // Take control of the port

	regCommand[0] = CMD_RREG | (address & 0x1F);
	regCommand[1] = 0;
	out_data_pntr = regCommand;
	out_data_count = 2;
	in_data_pntr = regCommand;
	in_data_count = 3;
	current_data_byte = 0;
	complete_callback = 0;

	PORTB &= 1;
	SPDR = regCommand[0];

	return;
}

uint8_t getReadRegister() {
	return regValue[0];
}

void startReadRegisters(uint8_t address, uint8_t count) {
	while (spi_busy)
	spi_busy = true; // Take control of the port

	regCommand[0] = CMD_RREG | (address & 0x1F);
	regCommand[1] = count - 1;
	out_data_pntr = regCommand;
	out_data_count = 2;
	in_data_pntr = regCommand;
	in_data_count = count + 2;
	current_data_byte = 0;
	complete_callback = 0;

	PORTB &= 1;
	SPDR = regCommand[0];

	return;
}

void getReadRegisters(uint8_t count, uint8_t *data) {
	memcpy(data, regValue, count);
	return;
}

// Sends start command. If SPI port is busy then busy waits to get access to the port
void sendCommand(uint8_t command) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	// Configure SPI transmit variables
	out_data_count = 1;
	in_data_count = 0;
	current_data_byte = 0;
	complete_callback = 0;

	PORTB &= ~(1); // Assert CS
	SPDR = command;

	// Leave the interrupts to do the rest
	return;
}

