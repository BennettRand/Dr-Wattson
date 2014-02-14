#include <avr/io.h>
#include <stdint.h>

#include "lcd.h"

struct lcd_cmd display_cmd_buffer[32];

int main(void) {
	initLCD(display_cmd_buffer, 32);
	DDRF &= ~(0b111);
	sendLCDCmd(LCD_CMD_CLEAR);
	serviceLCD(); // Clear the LCD just to make sure we know where we are.
	_delay_ms(2); // This takes a long time
	sendLCDCmd(LCD_CMD_DSP_ON);
	writeChar(0b01111111);
	writeChar(':');
	LCD_MOVE_TO_CHAR(0,4);
	writeChar(0b01111110);
	writeChar(':');
	LCD_MOVE_TO_CHAR(1,0);
	writeString("S:",2);

	for (int cnt = 0; cnt < 10; cnt++) {
		serviceLCD();
		_delay_us(60);
	}

	while (1) {
		LCD_MOVE_TO_CHAR(0,2);
		if (!(PINF & 0b10))
			writeChar('T');
		else
			writeChar('F');

		LCD_MOVE_TO_CHAR(0,6);
		if (!(PINF & 0b1))
			writeChar('T');
		else
			writeChar('F');

		LCD_MOVE_TO_CHAR(1,2);
		if (!(PINF & 0b100))
			writeChar('T');
		else
			writeChar('F');

		for (int cnt = 0; cnt < 10; cnt++) {
			serviceLCD();
			_delay_us(60);
		}
	}
}

