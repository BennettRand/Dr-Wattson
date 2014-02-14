#ifndef BASESTATION_H
#define BASESTATION_H

#include "sys.h"
#include "nwk.h"
#include "config.h"
#include "protocol.h"
#include <string.h>
#include "ui.h"

struct baseStation {
	uint16_t PAN_ID;
	int8_t rssi;
	uint16_t addr;
	char name[18];
	uint8_t nameLen;
};

extern struct baseStation baseStationList[BASESTATION_LIST_SIZE];
extern int8_t baseStationListLength;
extern int8_t connectedBaseStation;

int8_t addBaseStation(uint16_t PAN_ID, uint16_t addr, int8_t rssi, char name[]);
void removeBaseStation(int8_t num);
void processBaconPacket(NWK_DataInd_t *packet);
bool processConnectionAck(NWK_DataInd_t *packet);
void sendConnectionRequest(int8_t num, struct calibData *cal);

#endif //BASESTATION_H
