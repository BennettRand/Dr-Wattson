#ifndef _CONFIG_H_
#define _CONFIG_H_

/* file: config.h
 * This file provides the configuration provides the configuration definitions
 * for the Atmel Lightweight Mesh stack. The majority of these parameters are
 * compiled into the stack library at compile time, although a few are used by
 * the application program as well.
 */

#define APP_ENDPOINT              1
#define APP_FLUSH_TIMER_INTERVAL  20

#define APP_CHANNEL             0x08i

#define APP_SECURITY_KEY          ((uint8_t*) "Dr.WattspmtyKey0")
#define SYS_SECURITY_MODE                   1

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   1000 // ms

#define NWK_ENABLE_ROUTING
#define NWK_ENABLE_SECURITY

#endif // _CONFIG_H_
