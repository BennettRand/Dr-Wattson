#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

static uint16_t counter = 0;
static uint8_t count2 = 0;
static char transmitString[] = "I am Dr. Wattson:  \n";
static NWK_DataReq_t txPacket;

#define ANT_CHIP 0
#define ANT_EXT 1
void config_antenna(int ant) {
	// Set analog switch control pins as outputs
	DDRG |= 1<<1;
	DDRF |= 1<<2;

	if (ant == ANT_EXT) {
		PORTG &= ~(1<<1);
		PORTF |= 1<<2;
	}
	else if (ant == ANT_CHIP) {
		PORTG |= 1<<1;
		PORTF &= ~(1<<2);
	}
}

static void appDataConf(NWK_DataReq_t *req)
{
	// For now, we will do nothing
}
		
static void APP_TaskHandler(void)
{
	if (counter == 0) { 
		PORTB = (~PORTB)&(1<<4);
		transmitString[18] = count2 + '0';
		count2 = (count2+1)%10;
		
		txPacket.dstAddr = 3;
		txPacket.dstEndpoint = APP_ENDPOINT;
		txPacket.srcEndpoint = APP_ENDPOINT;
		txPacket.options = 0;
		txPacket.data = transmitString;
		txPacket.size = sizeof(transmitString)-1;
		txPacket.confirm = appDataConf;
		NWK_DataReq(&txPacket);
		
		counter = 0;
	}
	counter = (counter+((uint16_t)1))%300;
  // Put your application code here
}

int main(void)
{
  SYS_Init();
  DDRB |= 1<<4;
  PORTB |= 1<<4;
  DDRE |= 1<<2;
  
  config_antenna(ANT_CHIP);
  
  SYS_TaskHandler(); // Call the system task handler once before we configure the radio
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);
  PHY_SetTxPower(0);

  while (1)
  {
    SYS_TaskHandler();
    APP_TaskHandler();
	_delay_ms(1);
  }
}
