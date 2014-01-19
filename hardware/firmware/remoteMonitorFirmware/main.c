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

uint8_t uart_tx_buf[100];
uint8_t uart_rx_buf[100];
static NWK_DataReq_t nwkPacket;

static dataPacket_t dataPacket = {.type = data};
static uint8_t dataSequence = 0;

struct calibData deviceCalibration = {1, 2, 3, 4, 5, 6, 7, 8, 9};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" // Ignore the unused parameter warning here, the function has to have this prototype.
void packetTxConf(NWK_DataReq_t *req) {
	// We don't really care about if a packet was received or not
	// But we have to have a function here, because the library does not
	// handle null function pointers for the confirm callback.
}
#pragma GCC diagnostic pop

void handleDataRequest(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataRequestPacket_t))
		return; // Packet wrong size, just throw it out
	
	dataRequestPacket_t *reqPacket = (dataRequestPacket_t*)packet->data;

	dataSequence += 1;

	dataPacket.requestSequence = reqPacket->requestSequence;
	dataPacket.dataSequence = dataSequence;
	dataPacket.sampleCount = sampleCount;
	dataPacket.powerData = powerSum[0];
	#ifdef EXTENDED_DATA_PACKET
	dataPacket.linePeriod = linePeriod;
	dataPacket.squaredVoltage = voltageSum[0];
	dataPacket.squaredCurrent = currentSum[0];
	#endif

	nwkPacket.dstAddr = baseStationList[connectedBaseStation].addr;
	nwkPacket.dstEndpoint = APP_ENDPOINT;
	nwkPacket.srcEndpoint = APP_ENDPOINT;
	nwkPacket.data = (uint8_t *)(&dataPacket);
	nwkPacket.size = sizeof(dataPacket_t);
	nwkPacket.confirm = packetTxConf;
	NWK_DataReq(&nwkPacket);
}

void handleDataAck(NWK_DataInd_t *packet) {
	if (packet->size != sizeof(dataAckPacket_t))
		return; // Not the right size for this kind of packet

	dataAckPacket_t *ackPacket = (dataAckPacket_t*)packet->data;

	if (ackPacket->dataSequence == dataSequence) { // This is the ack for the most recent data
		cli(); // Don't interrupt while we are doing this. Is it really safe to do this?
		sampleCount -= dataPacket.sampleCount;
		powerSum[0] -= dataPacket.powerData;
		#ifdef EXTENDED_DATA_PACKET
		linePeriod -= dataPacket.linePeriod;
		voltageSum[0] -= dataPacket.squaredVoltage;		
		currentSum[0] -= dataPacket.squaredCurrent;
		#endif
		sei();
	}
}

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
			eeprom_update_byte((uint8_t*)26, 0);
		}
		break;
	case dataRequest:
		handleDataRequest(ind);
		break;
	case dataAck:
		handleDataAck(ind);
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

	// Read eeprom data
	eeprom_read_block(&deviceCalibration, (void*)8, sizeof(deviceCalibration));
	if (eeprom_read_byte((uint8_t*)26) == true) { // There is valid data in the network information struct
		eeprom_read_block(baseStationList, (void*)27, sizeof(struct baseStation));
		baseStationListLength += 1;
		sendConnectionRequest(0, &deviceCalibration);
	}	

	uart_init_port(uart_baud_115200, uart_tx_buf, 100, uart_rx_buf, 100); // Init uart
	initDataAck();
//	sei();
	startDataAck();


	while (1) {
		SYS_TaskHandler();
/*
		if (counter == 1000) {
			printf("Power 1: %lld, Power2: %lld\n", powerSum[0], powerSum[1]);
			counter = 0;
		}
		counter = counter+1;
*/
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
