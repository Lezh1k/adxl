#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "lpc824.h"
#include "lpc824_api.h"
#include "lpc824_api_uart.h"
#include "adxl363.h"

inline void config_pins();

#define F_CPU 12000000UL
#define USART0_BAUD_RATE 19200

int
main(void) {

  static uint8_t ch_xyz[3] = {'x', 'y', 'z'};
  int16_t xyz[3];
  register int32_t i;

  adxl_reset();

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

  while (1) {
    xyz[0] = adxl_X();
    xyz[1] = adxl_Y();
    xyz[2] = adxl_Z();

    for (i = 0; i < 3; ++i) {
      GPIO_B14 = 1;
      while(~USART0_STAT & UART_STAT_TXRDY);
      USART0_TXDAT = ch_xyz[i];
      while(~USART0_STAT & UART_STAT_TXIDLE);

      while(~USART0_STAT & UART_STAT_TXRDY);
      USART0_TXDAT = (uint8_t) (xyz[i] >> 8);
      while(~USART0_STAT & UART_STAT_TXIDLE);

      while(~USART0_STAT & UART_STAT_TXRDY);
      USART0_TXDAT = (uint8_t) (xyz[i] & 0x00ff);
      while(~USART0_STAT & UART_STAT_TXIDLE);

      while(~USART0_STAT & UART_STAT_TXRDY);
      USART0_TXDAT = ch_xyz[i];
      while(~USART0_STAT & UART_STAT_TXIDLE);
      GPIO_B14 = 0;
    }
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
config_pins() {
  /*base config*/
  SWM_PINENABLE0 &= ~(1 << 6); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 7); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 8); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 9); // CLKIN on pin PIO0_1

  /*spi0 config / adxl*/
  SWM_PINASSIGN3 = 0x0dffffff;  //SPI0_CLK -> PIO0_13
  SWM_PINASSIGN4 = 0xff000017 | //SPI0_MOSI -> PIO0_23
                   0xff001100 | //SPI0_MISO -> PIO0_17
                   0xff0c0000;  //SPI0_SSEL0 -> PIO0_12

  /*uart0 config*/
  SWM_PINASSIGN0 = (0x0b << 0)  | //U0_TXD -> PIO0_11
                   (0x0a << 8)  | //U0_RXD -> PIO0_10
                   (0x0f << 16) | //U0_RTS -> PIO0_14 . RTS should be used as DE.
                   (0xff << 24); //disable U0_DTS
                   /*(0x0f << 24) ; //U0_DTS -> PIO0_15*/

  GPIO_DIR0 |= (1 << 14); //RE to output
}

void
SystemInit (void) {
  SYSCON_MAINCLKSEL   = 0x00; //main clock source = IRC
  SYSCON_SYSAHBCLKDIV = 0x01; //divider 1
  SYSCON_MAINCLKUEN   = 0x01; //update main clock source

  SYSCON_CLKOUTSEL = 0x00; // select main clock as clock out source
  SYSCON_CLKOUTDIV = 0x01; // enable divider
  SYSCON_CLKOUTUEN = 0x01; // update clockout source
  config_pins();
}
//////////////////////////////////////////////////////////////////////////
