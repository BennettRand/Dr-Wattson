#include "adc.h"

// These two commands shouldn't be used with sendCommand() because they
// require multiple bytes to be sent/read.
#define CMD_RDATA   (0x12)
#define CMD_RREG    (0x20)
#define CMD_WREG    (0x40)

//------------ SPI Interrupt Communication Variables and ISR -----------------
static volatile bool spi_busy;
static volatile uint8_t spi_out_data[25];
static volatile uint8_t spi_in_data[25];
static volatile uint8_t spi_byte_count = 0;
static uint8_t *spi_rx_ptr = spi_in_data;
static uint8_t *spi_tx_ptr = spi_out_data;
static uint8_t current_data_byte = 0;

// SPI transmit complete interrupt
ISR(SPI_STC_vect, ISR_NAKED) {
	PORTE = 1;
	spi_in_data[current_data_byte++] = SPDR;
	if (spi_byte_count > current_data_byte)
		SPDR = spi_out_data[current_data_byte];
	else {
		PORTB |= 1;
		spi_busy = false;
	}
	PORTE = 0;
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

	DDRE = 1;

	// Configure ISP port
	SPCR = (1<<SPIE) | (1<<SPE) | (1<<MSTR) | (1<<CPHA);
	SPSR |= (1<<SPI2X);
}

void writeRegister(uint8_t address, uint8_t value) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	spi_out_data[0] = CMD_WREG | (address & 0x1F);
	spi_out_data[1] = 0;
	spi_out_data[2] = value;
	spi_byte_count = 3;
	current_data_byte = 0;

	PORTB &= ~1;
	SPDR = spi_out_data[0];

	return;
}

void writeRegisters(uint8_t address, uint8_t *data, uint8_t count) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	spi_out_data[0] = CMD_WREG | (address & 0x1F);
	spi_out_data[1] = count - 1;
	memcpy(spi_out_data + 2, data, count);
	spi_byte_count = 2 + count;
	current_data_byte = 0;

	PORTB &= ~1;
	SPDR = spi_out_data[0];

	return;
}

void startReadRegister(uint8_t address) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	spi_out_data[0] = CMD_RREG | (address & 0x1F);
	spi_out_data[1] = 0;
	spi_out_data[2] = 0;
	spi_byte_count = 3;
	current_data_byte = 0;

	PORTB &= ~1;
	SPDR = spi_out_data[0];

	return;
}

uint8_t getReadRegister() {
	return spi_in_data[2];
}

void startReadRegisters(uint8_t address, uint8_t count) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	spi_out_data[0] = CMD_RREG | (address & 0x1F);
	spi_out_data[1] = count - 1;
	memset(spi_out_data+2, 0, count);
	spi_byte_count = count + 2;
	current_data_byte = 0;

	PORTB &= ~1;
	SPDR = spi_out_data[0];

	return;
}

void getReadRegisters(uint8_t count, uint8_t *data) {
	memcpy(data, spi_in_data+2, count);
	return;
}

// Sends start command. If SPI port is busy then busy waits to get access to the port
void sendCommand(uint8_t command) {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	// Configure SPI transmit variables
	spi_byte_count = 1;
	current_data_byte = 0;

	PORTB &= ~(1); // Assert CS
	SPDR = command;

	// Leave the interrupts to do the rest
	return;
}

void readData() {
	while (spi_busy);
	spi_busy = true; // Take control of the port

	// Configure SPI transmit variables
	memset(spi_out_data, 0, 20);
	spi_byte_count = 11;
	current_data_byte = 0;

	PORTB &= ~(1); // Assert CS
	SPDR = CMD_RDATA;

	// Leave the interrupts to do the rest
	return;
}


