#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define RX_HEADER_SIZE (5)
typedef struct {
	uint16_t size;
	uint16_t sourceAddr;
	int8_t rssi;
} rxHeader_t;
	

#endif //PROTOCOL_H
