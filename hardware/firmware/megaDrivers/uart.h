#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#ifdef USE_STDIO
#include <stdio.h>
#endif

#include "config.h"

/* These values are only correct for a 16Mhz oscillator*/
typedef enum {
	uart_baud_2400 = 832,
	uart_baud_4800 = 416,
	uart_baud_9600 = 207,
	uart_baud_14400 = 138,
	uart_baud_19200 = 103,
	uart_baud_28800 = 68,
	uart_baud_38400 = 51,
	uart_baud_57600 = 34,
	uart_baud_76800 = 25,
	uart_baud_115200 = 16,
	uart_baud_230400 = 8,
	uart_baud_250000 = 7,
	uart_baud_500000 = 3,
	uart_baud_1M = 1
} uart_baud_t;

void uart_init_port(uart_baud_t baud_rate, void *tx_buf, uint16_t tx_buf_len, void *rx_buf, uint16_t rx_buf_len);
int uart_tx_data(void *data, uint16_t data_length);
int uart_tx_byte(uint8_t data);
int uart_rx_data(void *data, uint16_t data_length);
uint8_t uart_rx_byte(void);
uint16_t uart_received_bytes(void);
uint8_t uart_rx_peek(uint16_t byte);
void uart_rx_flush();

#endif //UART_H
