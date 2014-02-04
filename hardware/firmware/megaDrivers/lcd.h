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

typedef struct {
	uint8_t data;
	bool command;
} lcd_cmd_t;

void initLCD(lcd_cmd_t buffer[], uint8_t buf_len);
void serviceLCD(void);
void sendLCDCmd(uint8_t byte);
void writeChar(char c);
void writeString(char* c);

#endif //LCD_H
