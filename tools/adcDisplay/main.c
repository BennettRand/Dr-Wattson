#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "adc.h"
#include "lcd.h"

volatile int16_t inputBuffer[4];
uint16_t sampleCount = 0;
bool transmitting = false;
bool waiting = true;
struct lcd_cmd display_cmd_buffer[32];


ISR(PCINT0_vect) { // Data ready triggered
	readData(&(inputBuffer[0]),4);
	PCIFR = (1<<PCIF0); // Need to clear the pin change interrupt flag as we leave to clear out the rising edge interrupt on drdy.
}

int main(void) {
	initLCD(display_cmd_buffer, 32);
	sendLCDCmd(LCD_CMD_CLEAR);
	serviceLCD(); // Clear the LCD just to make sure we know where we are.
	_delay_ms(2); // This takes a long time
	sendLCDCmd(LCD_CMD_DSP_ON);

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

	sendCommand(CMD_START);
	sendCommand(CMD_RDATAC);
	PCICR |= 1;
	char buf[6];
	while (1) {
		itoa(inputBuffer[0], buf, 10);
		LCD_MOVE_TO_CHAR(0,0);
		writeString(buf, 6);
		for (int i = 0; i < 10; i++) {
			serviceLCD();
			_delay_us(60);
		}
		_delay_ms(100);
	}
}
