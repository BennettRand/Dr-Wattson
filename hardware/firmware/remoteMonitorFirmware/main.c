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

static NWK_DataReq_t txPacket;
uint8_t uart_tx_buf[200];
uint8_t uart_rx_buf[200];
extern struct baseStation baseStationList[BASESTATION_LIST_SIZE];
extern int8_t baseStationListLength;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" // Ignore the unused parameter warning here, the function has to have this prototype.
static void packetTxConf(NWK_DataReq_t *req) {
	// We don't really care about if a packet was received or not
	// But we have to have a function here, because the library does not
	// handle null function pointers for the confirm callback.
}
#pragma GCC diagnostic pop

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
	default:
		break;
	}

	return true;
}

int main(void) {
	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(boot_signature_byte_get(0x0100) | (((uint16_t)boot_signature_byte_get(0x0101)) << 8)); // Set network address based upon the MAC address
	NWK_SetPanId(0); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	//NWK_SetSecurityKey(APP_SECURITY_KEY);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	uart_init_port(uart_baud_38400, uart_tx_buf, 200, uart_rx_buf, 200); // Init uart

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
	}
}
