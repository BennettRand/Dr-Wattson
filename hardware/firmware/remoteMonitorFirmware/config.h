#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

/* file: config.h
 * This file provides the configuration provides the configuration definitions
 * for the Atmel Lightweight Mesh stack. The majority of these parameters are
 * compiled into the stack library at compile time, although a few are used by
 * the application program as well.
 */

struct calibData {
	uint16_t channel1VoltageScaling;
	int16_t channel1VoltageOffset;
	uint16_t channel1CurrentScaling;
	int16_t channel1CurrentOffset;
	uint16_t channel2VoltageScaling;
	int16_t channel2VoltageOffset;
	uint16_t channel2CurrentScaling;
	int16_t channel2CurrentOffset;
	uint16_t linePeriodScalingFactor;
};

extern struct calibData deviceCalibration;

#define EXTENDED_DATA_PACKET
#define DATA_REQ_BUFFER_CNT 32

#define USART_TX_vect USART0_TX_vect
#define USART_RX_vect USART0_RX_vect
#define USART_UDR     UDR0
#define USART_UBRR    UBRR0
#define USART_UCSRA   UCSR0A
#define USART_UCSRB   UCSR0B
#define USART_UCSRC   UCSR0C


#define BASESTATION_LIST_SIZE               8

#define APP_ENDPOINT                        1
#define APP_FLUSH_TIMER_INTERVAL            20

#define APP_CHANNEL                         0x08i

#define APP_SECURITY_KEY                    ((uint8_t*) "Dr.Wattson Key 0")
#define SYS_SECURITY_MODE                   0

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   1000 // ms

#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#endif // _CONFIG_H_
