#include <avr/io.h>
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
static uint8_t count2 = 0;
static char transmitString[] = "I am Dr. Wattson:  \n";
static NWK_DataReq_t txPacket;

uint8_t tx_buf[100];
uint8_t rx_buf[100];

static void packetTxConf(NWK_DataReq_t *req) {
	// We don't really care about if a packet was received or not
	// But we have to have a function here, because the library does not
	// handle null function pointers for the confirm callback.
}
		
static void APP_TaskHandler(void) {
	if (counter == 0) { 
		PORTB = (~PORTB)&(1<<4);
		transmitString[18] = count2 + '0';
		count2 = (count2+1)%10;
		
		txPacket.dstAddr = 0;
		txPacket.dstEndpoint = APP_ENDPOINT;
		txPacket.srcEndpoint = APP_ENDPOINT;
		txPacket.options = 0;
		txPacket.data = transmitString;
		txPacket.size = sizeof(transmitString);
		txPacket.confirm = packetTxConf;
		NWK_DataReq(&txPacket);
		
		counter = 0;
	}
	counter = (counter+((uint16_t)1))%400;
  // Put your application code here
}

static bool rfReceivePacket(NWK_DataInd_t *ind) {
	rxHeader_t packetHeader;
	packetHeader.size = ind->size + sizeof(rxHeader_t);
	packetHeader.sourceAddr = ind->srcAddr;
	packetHeader.rssi = ind->rssi;
	uart_tx_data((uint8_t*)(&packetHeader), sizeof(rxHeader_t));
	uart_tx_data(ind->data, ind->size);
}

int main(void) {
	SYS_Init(); // Init Atmel Lightweight Mesh stack
	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	uart_init_port(uart_baud_38400, tx_buf, 100, rx_buf, 100); // Init uart

	// Configure onboard LED as output
	DDRB |= 1<<4;
	PORTB |= 1<<4;

	// Configure analog switch for antenna
	DDRG |= 1<<1;
	DDRF |= 1<<2;
	PORTG |= 1<<1;
	PORTF &= ~(1<<2);
	
	while (1)
	{
	  PORTE |= 1<<2;
	  SYS_TaskHandler();
	  PORTE &= ~(1<<2);
	  APP_TaskHandler();
	  _delay_ms(1);
	}
}
