#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>

#include "config.h"

#include "lcd.h"

struct lcd_cmd display_cmd_buffer[32];


	uint16_t counter = 0;
int main(void) {
	// Init IO pins
	DDRG |= (1<<2) | (1<<3);
	PORTG |= (1<<2); // Clock should idle high because it is falling edge triggered.
	DDRB |= (1<<5);
	PORTB &= (1<<5); // Generally we want to be in write mode.
	DDRE = 0xFF; // Set data port to outputs
	_delay_ms(100);
	
	PORTE = 0b00110000;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_ms(5);

	PORTE = 0b00110000;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(200);

	PORTE = 0b00110000;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(200);

	PORTE = 0b00111000;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(100);

	PORTE = 0b00001000;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(100);

	PORTE = 0b00000001;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_ms(4);

	PORTE = 0b00000110;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(100);

	PORTE = 0b00001111;
	_delay_us(1);
	PORTB &= ~(1<<2);
	_delay_us(2);
	PORTB |= (1<<2);
	_delay_us(100);


	while (1) {
		_delay_us(60);
	}

}

