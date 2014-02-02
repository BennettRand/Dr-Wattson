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

#include "uart.h"
#include "protocol.h"
#include "basestation.h"
#include "dataAck.h"
#include "int_sqrt.h"

NWK_DataReq_t nwkPacket[DATA_REQ_BUFFER_CNT];
bool dataReqBusy[DATA_REQ_BUFFER_CNT];

uint8_t uart_tx_buf[100];
uint8_t uart_rx_buf[100];

static dataPacket_t dataPacket = {.type = data};
static uint8_t dataSequence = 0;

struct calibData deviceCalibration = {1, 2, 3, 4, 5, 6, 7, 8, 9};

void packetTxConf(NWK_DataReq_t *req) {
	dataReqBusy[req - nwkPacket] = false;
}

void handleDataRequest(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataRequestPacket_t))
		return; // Packet wrong size, just throw it out
	
	dataRequestPacket_t *reqPacket = (dataRequestPacket_t*)packet->data;

	dataSequence += 1;

	dataPacket.requestSequence = reqPacket->requestSequence;
	dataPacket.dataSequence = dataSequence;
	dataPacket.sampleCount = sampleCount;
	dataPacket.powerData1 = powerSum[0];
	dataPacket.powerData2 = powerSum[1];
	#ifdef EXTENDED_DATA_PACKET
	dataPacket.linePeriod = linePeriod;
	dataPacket.squaredVoltage1 = voltageSum[0];
	dataPacket.squaredVoltage2 = voltageSum[1];
	dataPacket.squaredCurrent1 = currentSum[0];
	dataPacket.squaredCurrent2 = currentSum[1];
	#endif

	uint8_t ind = 0;
	while (dataReqBusy[ind]) {
		SYS_TaskHandler();
		ind++;
	}
	nwkPacket[ind].dstAddr = baseStationList[connectedBaseStation].addr;
	nwkPacket[ind].dstEndpoint = APP_ENDPOINT;
	nwkPacket[ind].srcEndpoint = APP_ENDPOINT;
	nwkPacket[ind].options = 0;
	nwkPacket[ind].data = (uint8_t *)(&dataPacket);
	nwkPacket[ind].size = sizeof(dataPacket_t);
	nwkPacket[ind].confirm = packetTxConf;
	NWK_DataReq(&(nwkPacket[ind]));
	dataReqBusy[ind] = true;

//	printf("Voltage: %umV\n", (int_sqrt((uint32_t)(voltageSum[0]/((uint32_t)sampleCount)))*deviceCalibration.channel1VoltageScaling)/10000000);
//	printf("Power: %umw\n", (((int64_t)(powerSum[0]/10000000ll)*(int32_t)deviceCalibration.channel1VoltageScaling*(int32_t)deviceCalibration.channel1CurrentScaling)/sampleCount)/1000);
}

void handleDataAck(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataAckPacket_t))
		return; // Not the right size for this kind of packet

	dataAckPacket_t *ackPacket = (dataAckPacket_t*)packet->data;

	if (ackPacket->dataSequence == dataSequence) { // This is the ack for the most recent data
		cli(); // Don't interrupt while we are doing this. Is it really safe to do this?
		sampleCount -= dataPacket.sampleCount;
		powerSum[0] -= dataPacket.powerData1;
		powerSum[1] -= dataPacket.powerData2;
		#ifdef EXTENDED_DATA_PACKET
		linePeriod -= dataPacket.linePeriod;
		voltageSum[0] -= dataPacket.squaredVoltage1;		
		voltageSum[1] -= dataPacket.squaredVoltage2;		
		currentSum[0] -= dataPacket.squaredCurrent1;
		currentSum[1] -= dataPacket.squaredCurrent2;
		#endif
		sei();
	}
}

extern int16_t adcSampleData[4];
static bool rfReceivePacket(NWK_DataInd_t *ind) {
	// First figure out what kind of packet this is, and then call the appropreate function.
	switch ((packetType_t) (ind->data[0])) {
	case bacon:
		processBaconPacket(ind);
		printf("Got Bacon packet %d\n",baseStationListLength); 
		break;
	case connectionAck:
		if(processConnectionAck(ind)) {
			printf("Connected to network\n");
			eeprom_update_block(&(baseStationList[connectedBaseStation]), (void*)27, sizeof(struct baseStation));
			eeprom_update_byte((uint8_t*)29, 1); // Force the RSSI of the connected base station to 1 without modifying value in ram.
			eeprom_update_byte((uint8_t*)26, 0xFF);
		}
		break;
	case dataRequest:
		handleDataRequest(ind);
		break;
	case dataAck:
		handleDataAck(ind);
		break;
	case coldStart:
		sendConnectionRequest(connectedBaseStation, &deviceCalibration);
		printf("Got Cold Start\n");
		break;
	default:
		break;
	}

	return true;
}

	uint16_t counter = 0;
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
	uart_init_port(uart_baud_115200, uart_tx_buf, 100, uart_rx_buf, 100); // Init uart

	// Read eeprom data
	eeprom_read_block(&deviceCalibration, (void*)8, sizeof(deviceCalibration));
	if (eeprom_read_byte((uint8_t*)26)) { // There is valid data in the network information struct
		eeprom_read_block(baseStationList, (void*)27, sizeof(struct baseStation));
		baseStationListLength += 1;

		printf("Last connected network (%d): %u %u %16s\n", baseStationListLength, baseStationList[0].PAN_ID, baseStationList[0].addr, baseStationList[0].name);

		sendConnectionRequest(0, &deviceCalibration);
	}	

	initDataAck();
	sei();
	startDataAck();


	while (1) {
		SYS_TaskHandler();
		if (uart_received_bytes() != 0) {
			uint8_t data_byte = uart_rx_byte() - '0';
			if (data_byte < BASESTATION_LIST_SIZE) {
				sendConnectionRequest(data_byte, &deviceCalibration);
				printf("Connecting to network %u\n", data_byte);
			}
		}

//	EIMSK |= 1;
	}
}
