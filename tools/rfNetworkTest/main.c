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

#define DATA_REQ_BUFFER_CNT 32

static NWK_DataReq_t txPacket[DATA_REQ_BUFFER_CNT];
static bool dataReqBusy[DATA_REQ_BUFFER_CNT];

uint8_t uart_tx_buf[100];
uint8_t uart_rx_buf[100];
uint8_t packet_buf[100];

static void packetTxConf(NWK_DataReq_t *req) {
	dataReqBusy[req - txPacket] = false;
}
		
static bool rfReceivePacket(NWK_DataInd_t *ind) {
	uart_tx_data(ind->data, ind->size);
	uart_tx_byte('\n');
	return true;
}

int main(void) {
	uint16_t counter = 0;

	SYS_Init(); // Init Atmel Lightweight Mesh stack

	SYS_TaskHandler(); // Call the system task handler once before we configure the radio
	NWK_SetAddr(4);
	NWK_SetPanId(1); // Default PAN ID will be 0, can be changed using the set PAN command
	PHY_SetChannel(APP_CHANNEL);
	//NWK_SetSecurityKey(APP_SECURITY_KEY);
	PHY_SetRxState(true);
	NWK_OpenEndpoint(APP_ENDPOINT, rfReceivePacket);
	PHY_SetTxPower(0);

	uart_init_port(uart_baud_115200, uart_tx_buf, 100, uart_rx_buf, 100); // Init uart

	// Configure onboard LED as output
	//DDRB |= 1<<4;
	//PORTB |= 1<<4;

	// Configure analog switch for antenna
	//DDRG |= 1<<1;
	//DDRF |= 1<<2;
	//PORTG |= 1<<1;
	//PORTF &= ~(1<<2);

	char dat[100];
	uint8_t len;
	
	while (1) {
		SYS_TaskHandler();
		
		// Parse received uart packets, if we get one, then transmit it to the RF network
		if (uart_received_bytes() == 0) /* no bytes, just continue */
			continue;
		
		if ((uart_received_bytes() > 0) && (uart_rx_peek(uart_received_bytes()-1) == 0x0D)) {
			len = uart_received_bytes();
			uart_rx_data(dat, uart_received_bytes());
			
			uint8_t ind = 0;
			while (dataReqBusy[ind]) {
				SYS_TaskHandler();
				ind++;
			}

			txPacket[ind].options = NWK_OPT_BROADCAST_PAN_ID;

			txPacket[ind].dstAddr = 0xFFFF;
			txPacket[ind].dstEndpoint = APP_ENDPOINT;
			txPacket[ind].srcEndpoint = APP_ENDPOINT;
			txPacket[ind].data = dat;
			txPacket[ind].size = len-1;
			txPacket[ind].confirm = packetTxConf;
			NWK_DataReq(&(txPacket[ind]));
			dataReqBusy[ind] = true;
			
		}
	}
}
