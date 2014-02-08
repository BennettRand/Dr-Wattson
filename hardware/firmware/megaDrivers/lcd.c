#include "lcd.h"

static struct lcd_cmd *lcd_buf;
static uint8_t lcd_buf_start = 0;
static uint8_t lcd_buf_end = 0;
static uint8_t lcd_buf_size;

void initLCD(struct lcd_cmd buffer[], uint8_t buf_len) {
	// Init IO pins
	DDRG |= (1<<2) | (1<<3);
	PORTG |= (1<<2); // Clock should idle high because it is falling edge triggered.
	DDRB |= (1<<5);
	PORTB &= (1<<5); // Generally we want to be in write mode.
	DDRE = 0xFF; // Set data port to outputs

	lcd_buf = buffer;
	lcd_buf_size = buf_len;

	sendLCDCmd(0x38); // Configure LCD for 8 bit mode with two lines of text
	serviceLCD();
	_delay_ms(5);
	
}

void sendLCDCmd(uint8_t cmd) {
	lcd_buf[lcd_buf_end].data = cmd;
	lcd_buf[lcd_buf_end].command = true;
	lcd_buf_end = (lcd_buf_end < (lcd_buf_size-1)) ? lcd_buf_end+1 : 0;
}

void writeChar(char c) {
	lcd_buf[lcd_buf_end].data = c;
	lcd_buf[lcd_buf_end].command = false;
	lcd_buf_end = (lcd_buf_end < (lcd_buf_size-1)) ? lcd_buf_end+1 : 0;
}

void writeString(char* c) {
	uint8_t charCnt = 0;
	while (*c != 0) {
		if (charCnt >= 16)
			break; // Can't print more than 16 characters
		if (charCnt == 8)
			LCD_MOVE_TO_CHAR(1,0); // move to beginning of second line if longer than 8 chars

		writeChar(*(c++));
		charCnt++;
	}
}

void serviceLCD(void) {
	if (lcd_buf_start != lcd_buf_end) {
		if (lcd_buf[lcd_buf_start].command)
			PORTG &= ~(1<<3);
		else
			PORTG |= (1<<3);

		PORTE = lcd_buf[lcd_buf_start].data;
		asm volatile("nop");
		PORTG &= ~(1<<2); // Bring clock low for 1us
		_delay_loop_2(4);
		PORTG |= (1<<2);
		
		lcd_buf_start = (lcd_buf_start < (lcd_buf_size-1)) ? lcd_buf_start+1 : 0;
	}
}

