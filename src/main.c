#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "lpc824.h"
#include "adxl363.h"
#include "usart0.h"

static void config_pins();
/*interrupt vector*/
void SPI0_IRQHandler(void) { while(1) ; }                    // SPI0 controller
void SPI1_IRQHandler(void) { while(1) ; }                    // SPI1 controller
void UART0_IRQHandler(void) { while(1) ; }                   // UART0
void UART1_IRQHandler(void) { while(1) ; }                   // UART1
void UART2_IRQHandler(void) { while(1) ; }                   // UART2
void I2C1_IRQHandler(void) { while(1) ; }                    // I2C1 controller
void I2C0_IRQHandler(void) { while(1) ; }                    // I2C0 controller

void MRT_IRQHandler(void) { while(1) ; }                     // Multi-Rate Timer
void CMP_IRQHandler(void) { while(1) ; }                     // Comparator
void WDT_IRQHandler(void) { while(1) ; }                     // PIO1 (0:11)
void BOD_IRQHandler(void) { while(1) ; }                     // Brown Out Detect
void FLASH_IRQHandler(void) { while(1) ; }                   // FLASH controller
void WKT_IRQHandler(void) { while(1) ; }                     // Wakeup timer
void ADC_SEQA_IRQHandler(void) { while(1) ; }                // ADC SEQA
void ADC_SEQB_IRQHandler(void) { while(1) ; }                // ADC SEQB
void ADC_THCMP_IRQHandler(void) { while(1) ; }               // ADC Threashold Compare
void ADC_OVR_IRQHandler(void) { while(1) ; }                 // ADC Overrun
void DMA_IRQHandler(void) { while(1) ; }                     // DMA controller
void I2C2_IRQHandler(void) { while(1) ; }                    // I2C2 controller
void I2C3_IRQHandler(void) { while(1) ; }                    // I2C3 controller
void PININT0_IRQHandler(void) { while(1) ; }                 // PIO INT0
void PININT1_IRQHandler(void) { while(1) ; }                 // PIO INT1
void PININT2_IRQHandler(void) { while(1) ; }                 // PIO INT2
void PININT3_IRQHandler(void) { while(1) ; }                 // PIO INT3
void PININT4_IRQHandler(void) { while(1) ; }                 // PIO INT4
void PININT5_IRQHandler(void) { while(1) ; }                 // PIO INT5
void PININT6_IRQHandler(void) { while(1) ; }                 // PIO INT6
void PININT7_IRQHandler(void) { while(1) ; }                 // PIO INT7

volatile uint8_t on = 0;
void SCT_IRQHandler(void) { // Smart Counter Timer
  GPIO_B0 = ((on = !on) ? 0 : 1);
}

int
main(void) {

  static uint8_t ch_xyz[3] = {'x', 'y', 'z'};
  int16_t xyz[3];
  register int32_t i;

  adxl_reset();
  usart0_init();

  SYSCON_PRESETCTRL &= ~(1 << 8); //reset SCT timer
  SYSCON_SYSAHBCLKCTRL |= (1 << 8); //enable clock for SCT timer
  SYSCON_PRESETCTRL |= (1 << 8); //take SCT timer out of reset

  SCTIMER_CTRL |= (1 << 2); //halt SCT timer
  SCTIMER_CTRL |= (1 << 18);

  SCTIMER_CONFIG |= (1 << 0); //32-bit. The SCT operates as a unified 32-bit counter.
  SCTIMER_MATCH0 = 0x10000U; //set delay period
  SCTIMER_MATCHREL0 = 0x10000U; //reload delay

  SCTIMER_EV0_STATE = (1 << 0); //event 0 pushes us into state 0
  SCTIMER_EV0_CTRL = (1 << 0) | (1 << 12) | (1 << 14) | (1 << 15); //go to state [15..19] = 1

  SCTIMER_EV1_STATE = (1 << 1); //event 1 pushes us into state 1
  SCTIMER_EV1_CTRL = (1 << 0) | (1 << 12) | (1 << 14); //go to state 0 [15..19] = 0

  SCTIMER_OUT0_SET = (1 << 0);
  SCTIMER_OUT0_CLR = (1 << 1);

  SCTIMER_LIMIT_L = (1 << 0) | (1 << 1);

//  NVIC_ISER0 |= (1 << 9); //enable SCT timer interrupt
//  SCTIMER_EVEN |= (1 << 0); //enable interrupt

//  SCTIMER_CTRL &= ~(1 << 2);
//  SCTIMER_CTRL &= ~(1 << 18); //out of halt SCT timer

  while (1) {
    xyz[0] = adxl_X();
    xyz[1] = adxl_Y();
    xyz[2] = adxl_Z();

    for (i = 0; i < 3; ++i) {
      xyz[i]++;
      usart0_send_sync(ch_xyz[i]);
      usart0_send_sync((uint8_t) (xyz[i] >> 8));
      usart0_send_sync((uint8_t) (xyz[i] & 0x00ff));
      usart0_send_sync('\n');
    }

    GPIO_B0 = ((on = !on) ? 0 : 1);
    for (i = 0x100000; --i;) ;
    GPIO_B0 = ((on = !on) ? 0 : 1);
    for (i = 0x100000; --i;) ;
//    asm(" wfi ");
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
  GPIO_DIR0 |= (1 << 0); //RE to output PIO0_0 to output
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
