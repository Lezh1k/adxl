#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define F_CPU 12000000UL
#define UNUSED_ARG(x) ((void)x)

uint16_t crc16(uint8_t *data, uint8_t len);

#endif  // COMMONS_H
