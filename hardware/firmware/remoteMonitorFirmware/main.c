#include <avr/io.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>

#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"

#include "protocol.h"
#include "basestation.h"
#include "dataAck.h"
#include "ui.h"
#include "lcd.h"

NWK_DataReq_t nwkPacket[DATA_REQ_BUFFER_CNT];
bool dataReqBusy[DATA_REQ_BUFFER_CNT];
uint8_t retransmit_cnt[DATA_REQ_BUFFER_CNT];
uint16_t retransmit_time[DATA_REQ_BUFFER_CNT];

static dataPacket_t dataPacket = {.type = data};
static uint8_t dataSequence = 0;

struct calibData deviceCalibration;

struct lcd_cmd display_cmd_buffer[64];

void packetTxConf(NWK_DataReq_t *req) {
	retransmit_cnt[req - nwkPacket]++;
	if ((req->status != NWK_SUCCESS_STATUS) && (req->status != NWK_NO_ROUTE_STATUS) && (retransmit_cnt[req-nwkPacket] < 10)) {
		retransmit_time[req-nwkPacket] = TCNT3 + 15 + (rand()/(RAND_MAX/(128*retransmit_cnt[req - nwkPacket])));
	}
	else {
		dataReqBusy[req - nwkPacket] = false;
		retransmit_time[req - nwkPacket] = 0;
	}
}

void handleDataRequest(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataRequestPacket_t))
		return; // Packet wrong size, just throw it out

	if (dataReady()) {
		dataRequestPacket_t *reqPacket = (dataRequestPacket_t*)packet->data;
	
		dataSequence += 1;
		dataPacket.requestSequence = reqPacket->requestSequence;
		getData(&dataPacket);
	
		uint8_t ind = 0;
		while (dataReqBusy[ind]) {
			SYS_TaskHandler();
			ind = (ind < (DATA_REQ_BUFFER_CNT - 1)) ? (ind+1) : 0;
		}
		nwkPacket[ind].dstAddr = baseStationList[connectedBaseStation].addr;
		nwkPacket[ind].dstEndpoint = APP_ENDPOINT;
		nwkPacket[ind].srcEndpoint = APP_ENDPOINT;
		nwkPacket[ind].options = NWK_OPT_ACK_REQUEST;
		nwkPacket[ind].data = (uint8_t *)(&dataPacket);
		nwkPacket[ind].size = sizeof(dataPacket_t);
		nwkPacket[ind].confirm = packetTxConf;
		dataReqBusy[ind] = true;
		retransmit_cnt[ind] = 1;
		retransmit_time[ind] = TCNT3 + 5 + (rand()/(RAND_MAX/(128*retransmit_cnt[ind]))); // Retransmit the packet at a random time in the future
		ui_updatePowerValues(&dataPacket);
	}
}

void handleDataAck(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataAckPacket_t))
		return; // Not the right size for this kind of packet
	removeSamples(&dataPacket);
}

extern int16_t adcSampleData[4];
static bool rfReceivePacket(NWK_DataInd_t *ind) {
	// First figure out what kind of packet this is, and then call the appropreate function.
	switch ((packetType_t) (ind->data[0])) {
	case bacon:
		processBaconPacket(ind);
		break;
	case connectionAck:
		if(processConnectionAck(ind)) {
			ui_baseStationConnected();
			eeprom_update_block(&(baseStationList[connectedBaseStation]), (void*)27, sizeof(struct baseStation));
			eeprom_update_byte((uint8_t*)29, 1); // Force the RSSI of the connected base station to 1 without modifying value in ram.
			eeprom_update_byte((uint8_t*)26, 0xFF);
		}
		break;
	case dataRequest:
		handleDataRequest(ind);
		return false;
		break;
	case dataAck:
		handleDataAck(ind);
		break;
	case coldStart:
		sendConnectionRequest(connectedBaseStation, &deviceCalibration);
		ui_baseStationDisconnected();
		break;
	default:
		break;
	}

	return true;
}

	uint8_t counter = 0;
int main(void) {
	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(eeprom_read_word((uint16_t*)0));
	NWK_SetPanId(0); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	//NWK_SetSecurityKey(APP_SECURITY_KEY);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	// Seed the pseudorandom number generator
	srand(eeprom_read_word((uint16_t*)0));

	// Read eeprom data
	eeprom_read_block(&deviceCalibration, (void*)8, sizeof(deviceCalibration));
	if (eeprom_read_byte((uint8_t*)26)) { // There is valid data in the network information struct
		eeprom_read_block(baseStationList, (void*)27, sizeof(struct baseStation));
		uint8_t ch = 17;
		while ((baseStationList[0].name[ch] == ' ') || (baseStationList[0].name[ch] == '\0') || (baseStationList[0].name[ch] == 0xFF)) {
			baseStationList[0].name[ch] = ' ';
			ch -= 1;
		}
		baseStationList[0].nameLen = ch+1;
		baseStationListLength += 1;

		for (int cnt = 0; cnt < BASESTATION_LIST_SIZE; cnt++) {
			baseStationList[cnt].name[16] = ' ';
			baseStationList[cnt].name[17] = ' ';
		}
		sendConnectionRequest(0, &deviceCalibration);
	}	

	initDataAck();
	initLCD(display_cmd_buffer, 64);
	initUI();

	TCCR0B |= (1<<CS01);
	TCCR3B |= (1<<CS32) | (1<<CS30);

	sei();
	startDataAck();

	while (1) {
		SYS_TaskHandler();
		updateUI();

		if (sampleCount > 40000) {
			if (dataPacket.sampleCount != 0)
				removeSamples(&dataPacket); // If the last transmitted data has not been acked then first remove the old data.
			getData(&dataPacket); // Sample new data
			ui_updatePowerValues(&dataPacket); // Update display
			removeSamples(&dataPacket); // Get rid of these samples now
		}
		
		if (TCNT0 > 80) {
			serviceLCD();
			TCNT0 = 0;
		}

		TCCR3B &= ~((1<<CS32) | (1<<CS30));
		for (uint8_t cnt = 0; cnt < DATA_REQ_BUFFER_CNT; cnt++) {
			if (dataReqBusy[cnt] && (retransmit_time[cnt] != 0)) {
				if (retransmit_time[cnt] <= TCNT3) {
					NWK_DataReq(&(nwkPacket[cnt]));
					retransmit_time[cnt] = 0;
				}
				else
					retransmit_time[cnt] -= TCNT3;
			}
		}
		TCNT3 = 0;
		TCCR3B |= (1<<CS32) | (1<<CS30);
	}
}

