#ifndef DATA_ACK_H
#define DATA_ACK_H

#include <avr/interrupt.h>
#include <avr/io.h>

#include "adc.h"
#include "config.h"
#include "protocol.h"

extern uint32_t sampleCount;

void initDataAck();
void stopDataAck();
void startDataAck();
bool dataReady();
void getData(dataPacket_t *pkt);
void removeSamples(dataPacket_t *pkt);

#endif //DATA_ACK_H
