#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint16_t size;
	uint16_t sourceAddr;
	int8_t rssi;
} rxHeader_t;
	

#endif //PROTOCOL_H
