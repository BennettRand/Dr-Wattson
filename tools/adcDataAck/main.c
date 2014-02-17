#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "uart.h"
#include "adc.h"

uint8_t uart_tx_buf[100];
uint8_t uart_rx_buf[100];

int16_t inputBuffer[425][4];
uint16_t sampleCount = 0;
bool transmitting = false;
bool waiting = true;


ISR(PCINT0_vect) { // Data ready triggered
	readData(&(inputBuffer[sampleCount++][0]),4);
	PCIFR = (1<<PCIF0); // Need to clear the pin change interrupt flag as we leave to clear out the rising edge interrupt on drdy.
}

ISR(INT0_vect) {
	if ((sampleCount > 300) && (transmitting == false) && (waiting == false)) {
		// We should probably stop sampling now
		PCICR &= ~1;
		PCIFR = (1<<PCIF0);
		transmitting = true;
	}
	if ((transmitting == false) && (waiting == false))
		PCICR |= 1;
}

int main(void) {
	uart_init_port(uart_baud_115200, uart_tx_buf, 100, uart_rx_buf, 100); // Init uart

	// Configure the ADC
	initADC();
	sei();
	sendCommand(CMD_STOP);
	sendCommand(CMD_SDATAC);
	uint8_t registers[] = {0x10, 0x10, 0x10, 0x10, 0x90, 0x90, 0x90, 0x90};
	writeRegisters(0x05, registers, 8);

	// Configure data ready interrupt
	DDRB &= ~(1<<4);
	PCMSK0 = 1<<4;

	// Configure zero cross interrupt
	EICRA |= (1<<ISC01) | (1<<ISC00);
	DDRE |= 1<<2 | 1<<3;

	sendCommand(CMD_START);
	sendCommand(CMD_RDATAC);
	EIMSK |= 1;


	while (1) {
		if (transmitting){
			for (uint16_t cnt = 0; cnt < sampleCount; cnt++) {
				printf("%u, %d, %d, %d, %d\n", cnt, inputBuffer[cnt][0], inputBuffer[cnt][1], inputBuffer[cnt][2], inputBuffer[cnt][3]);
				_delay_ms(1);
			}
			printf("d\n");
			transmitting = false;
			waiting = true;
		}
		else if (waiting) {
			if (uart_rx_byte() == 's') {
				waiting = false;
				sampleCount = 0;
			}
			
		}
	}
}
