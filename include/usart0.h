#ifndef USART0_H
#define USART0_H

#include <stdint.h>

void usart0Init(void);
void usart0SendSync(uint8_t sb);
uint8_t usart0RecvSync(void);
void usart0SendArr(uint8_t* data, uint16_t len);

void usart0MbTsxHandle();

#endif  // USART0_H
