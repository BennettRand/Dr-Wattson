#include "uart.h"

// Define internal functions and variables, so they cannot be used outside the library

#ifdef USE_STDIO
/// Internal function used by printf for sending characters
static int _uart_stdio_put_char(char c, FILE *stream);

/// Internal function used by scanf to read characters out of the rx buffer
static int _uart_stdio_get_char(FILE *stream);

// static variables for use with printf
static FILE _uart_stdio_file = FDEV_SETUP_STREAM(_uart_stdio_put_char,_uart_stdio_get_char,_FDEV_SETUP_RW);
#endif

static uint8_t *tx_buffer;          /**< Pointer to the transmit buffer */
static uint16_t tx_buffer_size;     /**< Size of the transmit buffer */
static uint16_t tx_buffer_start;    /**< Beginning position of data in the transmit buffer */
static uint16_t tx_buffer_end;	    /**< Ending position of data in the transmit buffer */
static uint8_t	*rx_buffer;         /**< Pointer to the receive buffer */
static uint16_t rx_buffer_size;     /**< Size of the receive buffer */
static uint16_t rx_buffer_start;    /**< Beginning position of data in the receive buffer */
static uint16_t rx_buffer_end;	    /**< Ending position of data in the receive buffer */
static bool currently_transmitting; /**< True if the port is currently transmitting, set false when the buffer is emptied */

ISR(USART_TX_vect) {
	if (tx_buffer_start != tx_buffer_end) {
		/* There is actually data to send, so increment the start position and send that byte. Make sure to handle wrap arounds*/ 
		currently_transmitting = true; 
		USART_UDR = tx_buffer[tx_buffer_start];      /* write the data into the output buffer */ 
		tx_buffer_start = (tx_buffer_start+1)%tx_buffer_size; 
        }
	else 
		currently_transmitting = false; 
} 

ISR(USART_RX_vect) { 
	/* Check if the buffer is full, if it is, then throw out the oldest data by incrementing rx_buffer_start */ 
	if (((rx_buffer_start != 0) && (rx_buffer_end == rx_buffer_start-1)) || ((rx_buffer_start == 0) && (rx_buffer_end == rx_buffer_size-1))) { 
		/* There is not enough room, so make so space */ 
		rx_buffer_start = ((rx_buffer_start+1) % rx_buffer_size); 
	} 
	/* Now that we have space, copy the byte into the buffer and increment the end */ 
	rx_buffer[rx_buffer_end] = USART_UDR; 
	rx_buffer_end = ((rx_buffer_end+1) % rx_buffer_size); 
} 

void uart_init_port(uart_baud_t baud_rate, void *tx_buf, uint16_t tx_buf_len, void *rx_buf, uint16_t rx_buf_len) {
	DDRE |= (1<<1);
	DDRE &= ~(1<<0);
	PORTE &= ~(1<<0);

	USART_UBRR = baud_rate;
	USART_UCSRA |= (1<<U2X1);
	USART_UCSRB |= (1<<RXCIE1) | (1<<TXCIE1) | (1<<RXEN1) | (1<<TXEN1);
	USART_UCSRC |= (1<<UCSZ10) | (1<<UCSZ11);

	tx_buffer = tx_buf;
	tx_buffer_size = tx_buf_len;
	rx_buffer = rx_buf;
	rx_buffer_size = rx_buf_len;
	
	#ifdef USE_STDIO
	// If this port is to be used as the standard IO port for printf/scanf, then we need to set the _uart_stdio_port pointer
	stdout = &_uart_stdio_file;
	stdin = &_uart_stdio_file;
	#endif
}

int uart_tx_data(void *data, uint16_t data_length) {
	// Get the amount of buffer left
	uint16_t buffer_remaining;

	if (tx_buffer_end < tx_buffer_start)
		buffer_remaining = tx_buffer_start - tx_buffer_end - 1;
	else
		buffer_remaining = ((tx_buffer_size + tx_buffer_start) - tx_buffer_end) - 1;

	// now assign buffer remaining to data_length if it's smaller
	
	data_length = (buffer_remaining < data_length) ? buffer_remaining : data_length;

	if (data_length == 0)
		return 0;

	cli();
	// now copy the data into the buffer
	if (tx_buffer_start <= tx_buffer_end) { // The buffer has not wrapped around, so we have two cases
		if ((tx_buffer_size - tx_buffer_end) > data_length) { // All the data will fit at the end of the buffer
			memcpy(tx_buffer + tx_buffer_end, data, data_length);
		}
		else { // All the data won't fit at the end of the buffer, so we need to wrap the data around
			memcpy(tx_buffer + tx_buffer_end, data, (tx_buffer_size - tx_buffer_end));
			memcpy(tx_buffer, data + (tx_buffer_size - tx_buffer_end), data_length - (tx_buffer_size - tx_buffer_end));
		}
	}
	else { // The buffer has wrapped around, so there is only one way data can be put in
		memcpy(tx_buffer + tx_buffer_end, data, data_length);
	}

	tx_buffer_end = (tx_buffer_end + data_length) % tx_buffer_size;

	// If we were not in the middle of transmittind data, then we should start of the transmit by putting the first byte into the output buffer
	if (currently_transmitting == false) {
		USART_UDR = tx_buffer[tx_buffer_start];
		tx_buffer_start = ((tx_buffer_start + 1) % tx_buffer_size);
		currently_transmitting = true;
	}
	sei();

	// Return the number of bytes that were actually put into the buffer
	return data_length;
}

inline int uart_tx_byte(uint8_t data) {
	return uart_tx_data(&data, 1);
}

int uart_rx_data(void *data, uint16_t data_length) {
	uint16_t byte_cnt = 0;
	for (byte_cnt = 0; byte_cnt < data_length; byte_cnt++) {
		// check that there is actually more data to read
		if (rx_buffer_start == rx_buffer_end)
			// we are at the end of the buffer
			break;

		// if there is more data in the buffes, then get a byte
		((uint8_t*)data)[byte_cnt] = rx_buffer[rx_buffer_start];
		rx_buffer_start = ((rx_buffer_start+1)%rx_buffer_size);
	}
	return byte_cnt;
}

inline uint8_t uart_rx_byte(void) {
	uint8_t rx_data = 0;
	uart_rx_data(&rx_data,1);
	return rx_data;
}

uint16_t uart_received_bytes(void) {
	// Check if the buffer wraps around
	if (rx_buffer_start <= rx_buffer_end) {
		// Buffer doesn't wrap around
		return rx_buffer_end - rx_buffer_start;
	}
	else {
		// It does wrap around
		return (rx_buffer_size - rx_buffer_start) + rx_buffer_end;
	}
}

uint8_t uart_rx_peek(uint16_t byte) {
	return rx_buffer[(rx_buffer_start + byte) % rx_buffer_size];
}

#ifdef USE_STDIO
static int _uart_stdio_put_char(char c, FILE *stream) {
	// Since stdio functions don't convert newlines to cr-lf, and I am lazy and like to only use new lines, check and see if we need to and a carrage return.
	if (c == '\n')
		uart_tx_byte('\r');
	uart_tx_byte(c);
	return 0;
}

static int _uart_stdio_get_char(FILE *stream) {
	static uint8_t rx_data;
	if (uart_rx_data(&rx_data,1) == 0)
		// The buffer was empty, so return an end of file
		return _FDEV_EOF;

	// If we got this far, we have a character to return
	return rx_data;
}
#endif

