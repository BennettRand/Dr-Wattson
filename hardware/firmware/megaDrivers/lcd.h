#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define LCD_CMD_CLEAR 0x01
#define LCD_CMD_HOME 0x02
#define LCD_CMD_DSP_OFF 0x08
#define LCD_CMD_DSP_ON 0x0C
#define LCD_MOVE_TO_CHAR(row,col) (sendLCDCmd(0x80 | (col +(row<<6)))); 

struct lcd_cmd {
	uint8_t data;
	bool command;
};

void initLCD(struct lcd_cmd buffer[], uint8_t buf_len);
void serviceLCD(void);
void sendLCDCmd(uint8_t byte);
void writeChar(char c);
void writeString(char* c, int8_t len);
void defineLCDChar(uint8_t index, uint8_t bits[8]);

#endif //LCD_H
