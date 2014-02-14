#include "ui.h"

enum ui_state {
	connecting = 0,
	connected = 1
};

static enum ui_state currentState = connecting;

int8_t curDisplayedBasestation = 0;
uint8_t curStartChar = 0;

uint8_t lastButtonState;

uint8_t charArray[8];

void initUI(void) {
	DDRF &= ~(0b111);
	lastButtonState = PINF & 0b111;
	sendLCDCmd(LCD_CMD_CLEAR);
	serviceLCD(); // Clear the LCD just to make sure we know where we are.
	_delay_ms(2); // This takes a long time
	sendLCDCmd(LCD_CMD_DSP_ON);

	// Set up characters
	charArray[0] = 0b00100;
	charArray[1] = 0b01110;
	charArray[2] = 0b10101;
	charArray[3] = 0b00100;
	charArray[4] = 0b00100;
	charArray[5] = 0b10101;
	charArray[6] = 0b01110;
	charArray[7] = 0b00100;
	defineLCDChar(0, charArray);

	charArray[0] = 0b00100;
	charArray[1] = 0b01110;
	charArray[2] = 0b10101;
	charArray[3] = 0b00100;
	charArray[4] = 0b00100;
	charArray[5] = 0b00100;
	charArray[6] = 0b00100;
	charArray[7] = 0b00000;
	defineLCDChar(1, charArray);

	charArray[0] = 0b00000;
	charArray[1] = 0b00100;
	charArray[2] = 0b00100;
	charArray[3] = 0b00100;
	charArray[4] = 0b00100;
	charArray[5] = 0b10101;
	charArray[6] = 0b01110;
	charArray[7] = 0b00100;
	defineLCDChar(2, charArray);

	ui_baseStationDisconnected();
	
	// Set up timer for scrolling network names
	TCCR5B |= (1<<CS52) | (1<<CS50);
}

void updateUI(void) {
	if (currentState == connecting) {
		// Display list of networks to connect to
		uint8_t changedButtons = (lastButtonState ^ (PINF & 0b111)) & lastButtonState;
		if (changedButtons == 0b001) {
			if (curDisplayedBasestation > 0) {
				curDisplayedBasestation -= 1;
				LCD_MOVE_TO_CHAR(1,0);
				if (curDisplayedBasestation == 0)
					writeChar(2);
				else
					writeChar(0);
				writeString(baseStationList[curDisplayedBasestation].name, 7);
				curStartChar = 0;
				TCNT5 = 0;
			}
		}
		else if (changedButtons == 0b010) {
			if (curDisplayedBasestation < (baseStationListLength-1)) {
				curDisplayedBasestation += 1;
				LCD_MOVE_TO_CHAR(1,0);
				if (curDisplayedBasestation == (baseStationListLength-1))
					writeChar(1);
				else
					writeChar(0);
				writeString(baseStationList[curDisplayedBasestation].name, 7);
				curStartChar = 0;
				TCNT5 = 0;
			}
		}
		else if (changedButtons == 0b100) {
			sendConnectionRequest(curDisplayedBasestation, &deviceCalibration);
		}

		lastButtonState = PINF & 0b111;

		if (TCNT5 > 15625) { // 1 second
			if (baseStationList[curDisplayedBasestation].nameLen > 7) {
				if (++curStartChar >= (baseStationList[curDisplayedBasestation].nameLen +2))
					curStartChar = 0;
				LCD_MOVE_TO_CHAR(1,1);
				if ((curStartChar + 7) < (baseStationList[curDisplayedBasestation].nameLen +2))
					writeString(baseStationList[curDisplayedBasestation].name+curStartChar, 7);
				else {
					writeString(baseStationList[curDisplayedBasestation].name+curStartChar, (baseStationList[curDisplayedBasestation].nameLen+2)-curStartChar);
					writeString(baseStationList[curDisplayedBasestation].name, 7-((baseStationList[curDisplayedBasestation].nameLen+2)-curStartChar));
				}
			}
			
			TCNT5 = 0;
		}
	}
	else {
		
	}
}

void ui_baseStationConnected(void) {
	LCD_MOVE_TO_CHAR(0,0);
	writeString("1:  0.0W2:  0.0W", 16);

	currentState = connected;
	TCNT5 = 0;
}

void ui_updatePowerValues(int64_t ch1, int64_t ch2, uint32_t sampleCount){
	if (currentState == connected) {
		char pwrStr[6] = {0,0,0,0,0,0};
		ldiv_t res = ldiv(((((ch1*((int64_t)deviceCalibration.channel1VoltageScaling))/1000000)*((int64_t)deviceCalibration.channel1CurrentScaling))/sampleCount),10000000);
		if (res.rem > (10000000/2))
			res.quot++;
		ltoa(res.quot, pwrStr, 10);
		uint8_t len = strlen(pwrStr);
		LCD_MOVE_TO_CHAR(0, 2);
		while (len++ < 5)
			writeChar(' ');
		writeString(pwrStr, 5);
		
		memset(pwrStr, 0, 6);
		res = ldiv(((((ch2*((int64_t)deviceCalibration.channel2VoltageScaling))/1000000)*((int64_t)deviceCalibration.channel2CurrentScaling))/sampleCount),10000000);
		if (res.rem > (10000000/2))
			res.quot++;
		ltoa(res.quot, pwrStr, 10);	
		len = strlen(pwrStr);
		LCD_MOVE_TO_CHAR(1, 2);
		while (len++ < 5)
			writeChar(' ');
		writeString(pwrStr, 5);
	}
}

void ui_baseStationDisconnected(void) {
	LCD_MOVE_TO_CHAR(0,0);
	writeString("Network?",8);
	LCD_MOVE_TO_CHAR(1,0);
	
	if (baseStationListLength != 0) {
		curDisplayedBasestation = 0;
		if (baseStationListLength != 1)
			writeChar(2);
		else
			writeChar(0b01111110);
		writeString(baseStationList[0].name, 7);
	}
	else
		writeString("        ",8);

	currentState = connecting;
}

void ui_baseStationListChanged(int8_t modifiedEntry) {
	LCD_MOVE_TO_CHAR(1,0);
	if (baseStationListLength == 1)
		writeChar(0b01111110);
	else if (curDisplayedBasestation == 0)
		writeChar(2);
	else if ((baseStationListLength-1) > curDisplayedBasestation)
		writeChar(0);
	else
		writeChar(1);
	if (curDisplayedBasestation == modifiedEntry) {
		LCD_MOVE_TO_CHAR(1,1);
		writeString(baseStationList[curDisplayedBasestation].name, 7);
	}
	
	if (curDisplayedBasestation >= baseStationListLength) {
		curDisplayedBasestation = baseStationListLength-1;
		LCD_MOVE_TO_CHAR(1,0);
		writeChar(1);
		writeString(baseStationList[curDisplayedBasestation].name, 7);
	}
}

