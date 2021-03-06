#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>

#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"

#include "uart.h"

#include "protocol.h"
#include "../remoteMonitorFirmware/protocol.h"

#define DATA_REQ_BUFFER_CNT 32

static NWK_DataReq_t txPacket[DATA_REQ_BUFFER_CNT];
static bool dataReqBusy[DATA_REQ_BUFFER_CNT];

uint8_t uart_tx_buf[512];
uint8_t uart_rx_buf[512];
uint8_t packet_buf[128];

uint16_t currentPAN;

static void packetTxConf(NWK_DataReq_t *req) {
	dataReqBusy[req - txPacket] = false;
}
		
static bool rfReceivePacket(NWK_DataInd_t *ind) {
	rxHeader_t packetHeader;
	packetHeader.size = ind->size;
	packetHeader.sourceAddr = ind->srcAddr;
	packetHeader.rssi = ind->rssi;
	uart_tx_data((uint8_t*)(&packetHeader), sizeof(rxHeader_t));
	uart_tx_data(ind->data, ind->size);
	return true;
}

int main(void) {
	uint16_t counter = 0;

	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(eeprom_read_word((uint16_t*)0));
	NWK_SetPanId(0); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	//NWK_SetSecurityKey(APP_SECURITY_KEY);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);
	currentPAN = 0;

	uart_init_port(uart_baud_115200, uart_tx_buf, 512, uart_rx_buf, 512); // Init uart

	// Configure onboard LED as output
	DDRB |= 1<<4;
	PORTB |= 1<<4;

	// Configure analog switch for antenna
	DDRG |= 1<<1;
	DDRF |= 1<<2;
	PORTG |= 1<<1;
	PORTF &= ~(1<<2);
	
	while (1) {
		SYS_TaskHandler();
		
		// Flash the LED so we can tell if something has frozen up
		if (counter == 4092) {
			PORTB ^= (1<<4);
			counter = 0;
		}
		else
			counter ++;
			

		// Parse received uart packets, if we get one, then transmit it to the RF network
		if (uart_received_bytes() == 0) /* no bytes, just continue */
			continue;
		
		if (uart_received_bytes() > 0) {
			switch (uart_rx_peek(0)) {
			case sizeof(baconPacket_t):
			case sizeof(connectionRequestPacket_t):
			case sizeof(connectionAckPacket_t):
			case sizeof(dataRequestPacket_t):
			case sizeof(dataPacket_t):
				break; // If we are any of these lengths, then continue
			default: // Otherwise flush the buffers and continue
				uart_rx_flush();
				continue;
			}

			if ((uart_rx_peek(0) + sizeof(txHeader_t)) <= uart_received_bytes()) {
				txHeader_t packetHeader;
				uart_rx_data(&packetHeader, sizeof(txHeader_t));
				uart_rx_data(packet_buf, packetHeader.size);
	
				// Switch PANs if needed			
				if (packetHeader.PAN_ID != currentPAN) {
					NWK_SetPanId(packetHeader.PAN_ID);
					currentPAN = packetHeader.PAN_ID;
					SYS_TaskHandler();
				}
	
				uint8_t ind = 0;
				while (dataReqBusy[ind]) {
					SYS_TaskHandler();
					ind = (ind < (DATA_REQ_BUFFER_CNT-1)) ? (ind+1) : 0;
				}
	
				if (packetHeader.PANBroadcast == 0)
					txPacket[ind].options = 0;
				else
					txPacket[ind].options = NWK_OPT_BROADCAST_PAN_ID;
	
				txPacket[ind].dstAddr = packetHeader.destAddr;
				txPacket[ind].dstEndpoint = APP_ENDPOINT;
				txPacket[ind].srcEndpoint = APP_ENDPOINT;
				txPacket[ind].data = packet_buf;
				txPacket[ind].size = packetHeader.size;
				txPacket[ind].confirm = packetTxConf;
				NWK_DataReq(&(txPacket[ind]));
				dataReqBusy[ind] = true;
			}
		}
	}
}
