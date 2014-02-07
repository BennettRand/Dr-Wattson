#include "ui.h"

enum ui_state {
	connecting = 0,
	connected = 1
};

static enum ui_state currentState = connecting;

int8_t curDisplayedBasestation = 0;

uint8_t lastButtonState;

void initUI(void) {
	DDRF &= ~(0b111);
	lastButtonState = PINF & 0b111;
	_delay_ms(5);
	sendLCDCmd(0x30); // Configure LCD for 8 bit mode with two lines of text
	serviceLCD();
	_delay_us(200);
	sendLCDCmd(0x30); // Configure LCD for 8 bit mode with two lines of text
	serviceLCD();
	_delay_us(200);
	sendLCDCmd(0x30); // Configure LCD for 8 bit mode with two lines of text
	serviceLCD();
	_delay_us(50);
	sendLCDCmd(LCD_CMD_DSP_OFF);
	serviceLCD();
	_delay_us(50);
	sendLCDCmd(LCD_CMD_CLEAR);
	serviceLCD();
	_delay_ms(6);
	sendLCDCmd(0b110);
	serviceLCD();
	_delay_us(50);
	sendLCDCmd(LCD_CMD_DSP_ON);
	//writeString("Hello World");
}

void updateUI(void) {
	if (currentState == connecting) {
		// Display list of networks to connect to
		uint8_t changedButtons = (lastButtonState ^ (PINF & 0b111)) & lastButtonState;
		if (changedButtons == 0b001) {
			// Up button pressed
		}
		else if (changedButtons == 0b010) {
			// Down button pressed
		}
		else if (changedButtons == 0b100) {
			// Select Button pressed
		}

		lastButtonState = PINF & 0b111;
	}
	else {
		
	}
}

