#include <avr/io.h>
#include <avr/boot.h>
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

static uint16_t counter = 0;
static NWK_DataReq_t txPacket;

uint8_t uart_tx_buf[100];
uint8_t uart_rx_buf[100];
uint8_t packet_buf[100];

static void packetTxConf(NWK_DataReq_t *req) {
	// We don't really care about if a packet was received or not
	// But we have to have a function here, because the library does not
	// handle null function pointers for the confirm callback.
}
		
static void APP_TaskHandler(void) {
	if (counter == 0) { 
		PORTB = (~PORTB)&(1<<4);
	}
	counter = (counter+((uint16_t)1))%400;
  // Put your application code here
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
	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(boot_signature_byte_get(0x0100) | (((uint16_t)boot_signature_byte_get(0x0101)) << 8)); // Set network address based upon the MAC address
	NWK_SetPanId(0); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	uart_init_port(uart_baud_38400, uart_tx_buf, 100, uart_rx_buf, 100); // Init uart

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
		APP_TaskHandler();

		// Parse received uart packets, if we get one, then transmit it to the RF network
		if (uart_received_bytes() == 0) /* no bytes, just continue */
			continue;

		if ((uart_received_bytes() > 0) && ((uart_rx_peek(0) + sizeof(txHeader_t)) <= uart_received_bytes())) {
			txHeader_t packetHeader;
			uart_rx_data(&packetHeader, sizeof(txHeader_t));
			uart_rx_data(packet_buf, packetHeader.size);
		
			if (packetHeader.command == setPAN)
				PHY_SetPanId(*((uint16_t*)packet_buf));
			else {
				if (packetHeader.command == sendPacket)
					txPacket.options = 0;
				else
					txPacket.options |= NWK_OPT_BROADCAST_PAN_ID;

				txPacket.dstAddr = packetHeader.destAddr;
				txPacket.dstEndpoint = APP_ENDPOINT;
				txPacket.srcEndpoint = APP_ENDPOINT;
				txPacket.data = packet_buf;
				txPacket.size = packetHeader.size;
				txPacket.confirm = packetTxConf;
				NWK_DataReq(&txPacket);
			}
		}
	}
}
