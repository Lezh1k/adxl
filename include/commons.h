#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define F_CPU 12000000UL
#define UNUSED_ARG(x) ((void)x)

uint16_t crc16(uint8_t *data, uint8_t len);

//MEGAGLOBAL VARIABLE. AHTUNG
//will be used in main function
uint32_t SoftwareInterruptsFlag;

#define SINT_USART0_MB_TSX   (1 << 0);
#define SINT_USART0_RX_READY (1 << 1);
#define SINT_USART0_TX_READY (1 << 2);
#define SINT_USART0_TX_IDLE  (1 << 3);


#define SetSoftwareInt(x) ((SoftwareInterruptsFlag |= x))
#define ClrSoftwareInt(x) ((SoftwareInterruptsFlag &= ~(x)))

#endif  // COMMONS_H
