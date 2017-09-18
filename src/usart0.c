#include "usart0.h"
#include "lpc824.h"
#include "lpc824_api_uart.h"

#define F_CPU 12000000UL
#define USART0_BAUD_RATE 19200

void UART0_IRQHandler(void) {
  while(1) ;
}                   // UART0
//////////////////////////////////////////////////////////////////////////

void
usart0_init() {
  SWM_PINASSIGN0 = (0x0e << 0)  | //U0_TXD -> PIO0_14
                   (0x0b << 8)  | //U0_RXD -> PIO0_11
                   (0x0f << 16) | //U0_RTS -> PIO0_15 . RTS should be used as DE.
                   (0xff << 24);  //disable U0_DTS

  SYSCON_PRESETCTRL &= ~(1 << 3); //reset USART0
  SYSCON_SYSAHBCLKCTRL |= (1 << 14); //enable clock for USART0
  SYSCON_PRESETCTRL |= (1 << 3); //take USART0 out of reset
  NVIC_ISER0 |= (1 << 3); //enable USART0 IRQ

  SYSCON_UARTCLKDIV = 0x01; //uart clock divider is 1. result 12 MHz
  USART0_BRG = F_CPU / (USART0_BAUD_RATE * 16); //got from documentation.

  USART0_CFG = (1 << 0) ; //enable usart0
  USART0_CFG |= (0x01 << 2); //8 bit Data length
  //  USART0_CFG &= ~(0x03 << 4); //no parity
  //  USART0_CFG |= (1 << 6); //2 stop bit
  //  USART0_CFG &= ~(1 << 11); //asynchronous mode is select

  GPIO_DIR0 |= (1 << 10); //PIO0_10 to output
  USART0_CFG |= (1 << 18); //Output Enable Turnaround time enable for RS-485 operation
  USART0_CFG |= (1 << 20); //output enable select
  USART0_CFG |= (1 << 21); //output enable polarity

  GPIO_B10 = 0; //RE always should be 0. DON'T REMOVE!!!!!!!!!!
}
//////////////////////////////////////////////////////////////////////////

void
usart0_send_sync(uint8_t sb) {
  while(~USART0_STAT & UART_STAT_TXRDY);
  USART0_TXDAT = sb;
  while(~USART0_STAT & UART_STAT_TXIDLE);  
}
//////////////////////////////////////////////////////////////////////////

uint8_t
usart0_recv_sync() {
  uint8_t sb = 0;  
  while(~USART0_STAT & UART_STAT_RXRDY);
  sb = USART0_RXDAT;
  while(~USART0_STAT & UART_STAT_TXIDLE);
  return sb;
}
//////////////////////////////////////////////////////////////////////////
