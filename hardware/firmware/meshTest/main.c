/**
 * \file template.c
 *
 * \brief Empty application template
 *
 * Copyright (C) 2012-2013, Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * $Id: template.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/*- Definitions ------------------------------------------------------------*/
// Put your preprocessor definitions here

/*- Types ------------------------------------------------------------------*/
// Put your type definitions here

/*- Prototypes -------------------------------------------------------------*/
// Put your function prototypes here

/*- Variables --------------------------------------------------------------*/
// Put your variables here
	static uint16_t counter = 0;
	static uint8_t count2 = 0;
	static char transmitString[] = "I am Dr. Wattson:  \n\r";
	static NWK_DataReq_t txPacket;
/*- Implementations --------------------------------------------------------*/

// Put your function implementations here
//static SYS_Timer_t hrtbtTimer;/*************************************************************************//**
//*****************************************************************************/

#define ANT_CHIP 0
#define ANT_EXT 1
void config_antenna(int ant) {
	// Set analog switch control pins as outputs
	DDRG |= 1<<1;
	DDRF |= 1<<2;

	if (ant == ANT_CHIP) {
		PORTG &= ~(1<<1);
		PORTF |= 1<<2;
	}
	else if (ant == ANT_EXT) {
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
		
		txPacket.dstAddr = 0;
		txPacket.dstEndpoint = APP_ENDPOINT;
		txPacket.srcEndpoint = APP_ENDPOINT;
		txPacket.options = NWK_OPT_ENABLE_SECURITY;
		txPacket.data = transmitString;
		txPacket.size = sizeof(transmitString);
		txPacket.confirm = appDataConf;
		NWK_DataReq(&txPacket);
		
		counter = 0;
	}
	counter = (counter+((uint16_t)1))%0xFFF;
  // Put your application code here
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
  HAL_Init();
  SYS_Init();
  DDRB |= 1<<4;
  PORTB |= 1<<4;
  
  config_antenna(ANT_EXT);
  
  SYS_TaskHandler(); // Call the system task handler once before we configure the radio
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);

  while (1)
  {
    SYS_TaskHandler();
    APP_TaskHandler();
  }
}
