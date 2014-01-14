#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CMD_WAKEUP  (0x02)
#define CMD_STANDBY (0x04)
#define CMD_RESET   (0x06)
#define CMD_START   (0x08)
#define CMD_STOP    (0x0A)
#define CMD_RDATAC  (0x10)
#define CMD_SDATAC  (0x11)

void initADC(); // Initializes IO and SPI peripherals for ADC communication
void writeRegister(uint8_t address, uint8_t value);
void writeRegisters(uint8_t address, uint8_t *data, uint8_t count);
void startReadRegister(uint8_t address);
uint8_t getReadRegister();
void startReadRegisters(uint8_t address, uint8_t count);
void getReadRegisters(uint8_t count, uint8_t *data); // Retrieves read register data from input buffer
void sendCommand(uint8_t command);

#endif //ADC_H
