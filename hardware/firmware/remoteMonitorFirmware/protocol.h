#ifndef PROTOCOL_COMM_H
#define PROTOCOL_COMM_H

#include <stdint.h>

#ifdef __AVR__
#include "config.h"
#endif

typedef enum __attribute__((packed)) {
	bacon = 0,
	connectionRequest = 1,
	connectionAck = 2,
	dataRequest = 3,
	data = 4,
	dataAck = 5
} packetType_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
	uint16_t PAN_ID;
	char name[16];
} baconPacket_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
	uint16_t channel1VoltageScaling;
	uint16_t channel1CurrentScaling;
	uint16_t channel2VoltageScaling;
	uint16_t channel2CurrentScaling;
	uint16_t linePeriodScalingFactor;
} connectionRequestPacket_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
} connectionAckPacket_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
	uint8_t requestSequence;
} dataRequestPacket_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
	uint8_t requestSequence;
	uint8_t dataSequence;
	uint32_t sampleCount;
	int64_t powerData;
	#ifdef EXTENDED_DATA_PACKET
	int16_t linePeriod;
	int64_t squaredVoltage;
	int64_t squaredCurrent;
	#endif
} dataPacket_t;

typedef struct __attribute__((packed)) {
	packetType_t type;
    uint8_t dataSequence;
} dataAckPacket_t;


#endif //PROTOCOL_COMM_H
