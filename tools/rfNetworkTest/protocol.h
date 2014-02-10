#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint8_t size;
	uint16_t sourceAddr;
	int8_t rssi;
} rxHeader_t;

enum __attribute__((packed)) tx_command {
	sendPacket,
	broadcastPacket,
	setPAN
};

typedef struct __attribute__((packed)) {
	uint8_t size;
	enum tx_command command;
	uint16_t destAddr;
} txHeader_t;

#endif //PROTOCOL_H
