#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "lpc824.h"
#include "adxl363.h"
#include "usart0.h"
#include "modbus_rtu_client.h"
#include "heap_memory.h"

static void configPins();

/*interrupt vector*/
void SPI0_IRQHandler(void) { while(1) ; }                    // SPI0 controller
void SPI1_IRQHandler(void) { while(1) ; }                    // SPI1 controller
//
//void UART0_IRQHandler(void) - see in usart0.c
void UART1_IRQHandler(void) { while(1) ; }                   // UART1
void UART2_IRQHandler(void) { while(1) ; }                   // UART2
//void I2C1_IRQHandler(void) { while(1) ; }                    // I2C1 controller
void I2C0_IRQHandler(void) { while(1) ; }                    // I2C0 controller

void SCT_IRQHandler(void) { while(1); }                       // Smart Counter Timer
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
//////////////////////////////////////////////////////////////////////////

typedef enum adxl_holding_settings{
  s_range = 0,
  s_odr,
  s_win_size,
  s_count
}adxl_holding_settings_t;
#define DEFAULT_WINDOW_SIZE 4

static uint8_t coilsBuff[24] = {0};
static uint8_t inputDiscreteBuff[24] = {0};
static uint16_t inputRegisters[256] = {0};
static uint16_t holdingRegisters[s_count] = {adxlr_2g, odr_100, DEFAULT_WINDOW_SIZE};
static mb_client_device_t m_device;

int
main(void) {  
  register uint8_t i;

  m_device.address = 2;
  m_device.coilsMap.startAddr = 0;
  m_device.coilsMap.realAddr = coilsBuff;
  m_device.coilsMap.endAddr = sizeof(coilsBuff);
  m_device.inputDiscreteMap.startAddr = 0;
  m_device.inputDiscreteMap.realAddr = inputDiscreteBuff;
  m_device.inputDiscreteMap.endAddr = sizeof(inputDiscreteBuff);
  m_device.holdingRegistersMap.start_addr = 0;
  m_device.holdingRegistersMap.real_addr = holdingRegisters;
  m_device.holdingRegistersMap.end_addr = sizeof(holdingRegisters);
  m_device.inputRegistersMap.start_addr = 0;
  m_device.inputRegistersMap.real_addr = inputRegisters;
  m_device.inputRegistersMap.end_addr = sizeof(inputRegisters);
  m_device.tp_send = usart0SendArr;

  hm_init();
  adxl_reset();
  usart0Init();

  mb_init(&m_device);
  while (1) {
    inputRegisters[0] = inputRegisters[1] = inputRegisters[2] = 0;
    for (i = 0; i < holdingRegisters[s_win_size]; ++i) {
      inputRegisters[0] += adxl_X() / holdingRegisters[s_win_size];
      inputRegisters[1] += adxl_Y() / holdingRegisters[s_win_size];
      inputRegisters[2] += adxl_Z() / holdingRegisters[s_win_size];
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
configPins() {
  GPIO_DIR0 |= (1 << 0); //PIO0_0 to output
}
//////////////////////////////////////////////////////////////////////////



void
systemInit (void) {
  /*in the IOCON block, remove the pull-up and pull-down resistors in the IOCON
registers for pins PIO0_8 and PIO0_9.*/
  IOCON_PIO0_8 &= (0x03 << 3); //no pull-up and pull-down
  IOCON_PIO0_9 &= (0x03 << 3); //no pull-up and pull-down

  /*In the switch matrix block, enable the 1-bit functions for XTALIN and XTALOUT.*/
  SWM_PINENABLE0 &= ~(1 << 6); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 7); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 8); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 9); // CLKIN on pin PIO0_1

  /* In the SYSOSCCTRL register, disable the BYPASS bit and select the oscillator
  frequency range according to the desired oscillator output clock.*/
  SYSCON_SYSOSCCTRL &= ~(1 << 0); //disable BYPASS
  SYSCON_SYSOSCCTRL &= ~(1 << 1); //0 <= FREQRANGE <= 20MHz
  SYSCON_SYSAHBCLKDIV = 0x01; //divider 1
  SYSCON_MAINCLKUEN   = 0x01; //update main clock source

  SYSCON_CLKOUTSEL = 0x00; // select main clock as clock out source
  SYSCON_CLKOUTDIV = 0x01; // enable divider
  SYSCON_CLKOUTUEN = 0x01; // update clockout source

  configPins();
}
//////////////////////////////////////////////////////////////////////////
