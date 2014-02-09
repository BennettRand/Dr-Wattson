#include <avr/io.h>
#include <stdint.h>

#include "lcd.h"

struct lcd_cmd display_cmd_buffer[32];

int main(void) {
	initLCD(display_cmd_buffer, 32);
	sendLCDCmd(LCD_CMD_CLEAR);
	serviceLCD(); // Clear the LCD just to make sure we know where we are.
	_delay_ms(2); // This takes a long time
	sendLCDCmd(LCD_CMD_DSP_ON);
	writeString("Line:1..Line:2..",16);

	while (1) {
		serviceLCD();
		_delay_us(60);

	}
}

