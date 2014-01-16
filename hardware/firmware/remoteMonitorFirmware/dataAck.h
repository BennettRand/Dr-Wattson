#ifndef DATA_ACK_H
#define DATA_ACK_H

#include <avr/interrupt.h>
#include <avr/io.h>

#include "adc.h"
#include "config.h"

extern uint16_t sampleCount;
extern int64_t powerSum[2];

#ifdef EXTENDED_DATA_PACKET
extern int64_t voltageSum[2];
extern int64_t currentSum[2];
extern uint16_t linePeriod[2];
#endif

void initDataAck();
void stopDataAck();
void StartDataAck();

#endif //DATA_ACK_H
