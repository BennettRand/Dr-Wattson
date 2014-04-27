#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint8_t size;
	uint16_t sourceAddr;
	int8_t rssi;
} rxHeader_t;

typedef struct __attribute__((packed)) {
	uint8_t size;
	uint8_t PANBroadcast;
	uint16_t PAN_ID;
	uint16_t destAddr;
} txHeader_t;

#endif //PROTOCOL_H
