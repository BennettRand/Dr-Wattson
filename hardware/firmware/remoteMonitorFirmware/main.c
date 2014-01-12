#include <avr/io.h>
#include <avr/boot.h>
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

static NWK_DataReq_t nwkPacket;

static dataPacket_t dataPacket = {.type = data};
static uint8_t dataSequence = 0;

uint16_t sampleCount;
int64_t powerSum;

#ifdef EXTENDED_DATA_PACKET
int64_t voltageSum;
int64_t currentSum;
uint16_t linePeriod;
#endif

struct calibData unitCal = {1, 2, 3, 4, 5};
uint8_t uart_tx_buf[200];
uint8_t uart_rx_buf[200];

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
	dataPacket.powerData = powerSum;
	#ifdef EXTENDED_DATA_PACKET
	dataPacket.linePeriod = linePeriod;
	dataPacket.squaredVoltage = voltageSum;
	dataPacket.squaredCurrent = currentSum;
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
		powerSum -= dataPacket.powerData;
		#ifdef EXTENDED_DATA_PACKET
		linePeriod -= dataPacket.linePeriod;
		voltageSum -= dataPacket.squaredVoltage;		
		currentSum -= dataPacket.squaredCurrent;
		#endif
		sei();
	}
}

static bool rfReceivePacket(NWK_DataInd_t *ind) {
	// First figure out what kind of packet this is, and then call the appropreate function.
	switch ((packetType_t) (ind->data[0])) {
	case bacon:
		processBaconPacket(ind);
		for (int8_t cnt = 0; cnt < baseStationListLength; cnt++) {
			printf("%u %16s\n", cnt, baseStationList[cnt].name);
			printf("   Addr: %u\n   PAN ID: %u\n   rssi: %d\n", baseStationList[cnt].addr, baseStationList[cnt].PAN_ID, baseStationList[cnt].rssi);
		}
		printf("\n");
		break;
	case connectionAck:
		if (processConnectionAck(ind))
			printf("Connected to network: %d\n", connectedBaseStation);
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

int main(void) {
	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	//NWK_SetAddr(boot_signature_byte_get(0x0100) | (((uint16_t)boot_signature_byte_get(0x0101)) << 8)); // Set network address based upon the MAC address
	NWK_SetAddr(4);
	NWK_SetPanId(0); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	//NWK_SetSecurityKey(APP_SECURITY_KEY);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	//uart_init_port(uart_baud_38400, uart_tx_buf, 200, uart_rx_buf, 200); // Init uart

	// Configure onboard LED as output
	//DDRB |= 1<<4;
	//PORTB |= 1<<4;

	// Configure analog switch for antenna
	//DDRG |= 1<<1;
	//DDRF |= 1<<2;
	//PORTG |= 1<<1;
	//PORTF &= ~(1<<2);
	uint16_t cnt = 0;
	while (1) {
		SYS_TaskHandler();
		if (cnt == 1000) {
			nwkPacket.dstAddr = 1;
			nwkPacket.dstEndpoint = APP_ENDPOINT;
			nwkPacket.srcEndpoint = APP_ENDPOINT;
			nwkPacket.data = "Hello World";
			nwkPacket.size = 11;
			nwkPacket.confirm = packetTxConf;
			NWK_DataReq(&nwkPacket);
			cnt = 0;
		}
		cnt = cnt+1;

	/*	if (uart_received_bytes() != 0) {
			uint8_t data_byte = uart_rx_byte() - '0';
			if (data_byte < BASESTATION_LIST_SIZE) {
				sendConnectionRequest(data_byte, &unitCal);
				printf("Connecting to network %u\n", data_byte);
			}
		}*/

		_delay_ms(1);
	}
}
