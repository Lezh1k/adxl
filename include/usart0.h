#ifndef USART0_H
#define USART0_H

#include <stdint.h>

void usart0_init(void);
void usart0_send_sync(uint8_t sb);

#endif  // USART0_H
