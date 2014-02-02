#include "lcd.h"

void initLCD() {
	// Init IO pins
	DDRG |= (1<<2) | (1<<3);
	DDRE = 0xFF; // Data port
	DDRB |= (1<<5);

	PORTB &= (1<<5);
}

