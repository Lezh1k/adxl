#include "usart0.h"
#include "lpc824.h"
#include "lpc824_api_uart.h"

#define F_CPU 12000000UL
#define USART0_BAUD_RATE 19200

void
usart0_init() {
  SWM_PINASSIGN0 = (0x0e << 0)  | //U0_TXD -> PIO0_14
                   (0x0b << 8)  | //U0_RXD -> PIO0_11
                   (0x0f << 16) | //U0_RTS -> PIO0_15 . RTS should be used as DE.
                   (0xff << 24); //disable U0_DTS

  SYSCON_PRESETCTRL &= ~(1 << 3); //reset USART0
  SYSCON_SYSAHBCLKCTRL |= (1 << 14); //enable clock for USART0
  SYSCON_PRESETCTRL |= (1 << 3); //take USART0 out of reset
  NVIC_ISER0 |= (1 << 3); //enable USART0 IRQ

  SYSCON_UARTCLKDIV = 0x01; //uart clock divider is 1. result 12 MHz
  USART0_BRG = F_CPU / (USART0_BAUD_RATE * 16); //o_O они убили кенни
  USART0_CFG = 0x01; //enable usart0

  USART0_CFG |=  (1 << 2); //8 bit Data length
  USART0_CFG &= ~(1 << 4); //no parity
  USART0_CFG &= ~(1 << 6); //1 stop bit
  USART0_CFG &= ~(1 << 11); //asynchronous mode is select
  USART0_CFG |= (1 << 18); //Output Enable Turnaround time enable for RS-485 operation
  USART0_CFG |= (1 << 20); //output enable select
  USART0_CFG |= (1 << 21); //output enable polarity
}
//////////////////////////////////////////////////////////////////////////

void
usart0_send_sync(uint8_t sb) {
  while(~USART0_STAT & UART_STAT_TXRDY);
  USART0_TXDAT = sb;
  while(~USART0_STAT & UART_STAT_TXIDLE);
}
//////////////////////////////////////////////////////////////////////////
