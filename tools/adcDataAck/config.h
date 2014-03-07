#ifndef _CONFIG_H_
#define _CONFIG_H_

#define BOARD_REV 2
 
#if (BOARD_REV != 1) && (BOARD_REV != 2)
	#error Invalid board revision definition.
#endif

#if (BOARD_REV == 1)
	#define USART_TX_vect USART0_TX_vect
	#define USART_RX_vect USART0_RX_vect
	#define USART_UDR     UDR0
	#define USART_UBRR    UBRR0
	#define USART_UCSRA   UCSR0A
	#define USART_UCSRB   UCSR0B
	#define USART_UCSRC   UCSR0C
#elif (BOARD_REV == 2)
	#define USART_TX_vect USART1_TX_vect
	#define USART_RX_vect USART1_RX_vect
	#define USART_UDR     UDR1
	#define USART_UBRR    UBRR1
	#define USART_UCSRA   UCSR1A
	#define USART_UCSRB   UCSR1B
	#define USART_UCSRC   UCSR1C
#endif

#endif // _CONFIG_H_
