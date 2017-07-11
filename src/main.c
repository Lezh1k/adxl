#include <stdint.h>
#include <stdlib.h>
#include "ARMCM0plus.h"
#include "lpc824.h"

//typedef struct {
//const uint32_t reserved0; /*!< Reserved */
//const uint32_t reserved1; /*!< Reserved */
//const uint32_t reserved2; /*!< Reserved */
//const PWRD_API_T *pPWRD; /*!< Power API function table base address */
//const ROM_DIV_API_T *divApiBase; /*!< Divider API function table base address */
//const I2CD_API_T *pI2CD;/*!< I2C driver routines functions table */
//const uint32_t reserved5; /*!< Reserved */
//const SPID_API_T *pSPID; /*!< SPI driver API function table base address */
//const ADCD_API_T *pADCD; /*!< ADC driver API function table base address */
//const UARTD_API_T *pUARTD; /*!< USART driver API function table base address */
//} LPC_ROM_API_T;
//#define ROM_DRIVER_BASE (0x1FFF1FF8UL)

int
main(void) {
  volatile int32_t i;
  volatile int32_t j;

  while (1) {
    PIO0_0 = 1;
    for (i = 0; i < 100000; ++i) {
      for (j = 0; j < 100000; ++j) {
        /*do nothing*/
      }
    }
    PIO0_0 = 0;
    for (i = 0; i < 100000; ++i) {
      for (j = 0; j < 100000; ++j) {
        /*do nothing*/
      }
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
SystemInit (void) {
  SWM_PINENABLE0 &= ~(1 << 6); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 7); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 8); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 9); // CLKIN on pin PIO0_1

  SYSCON_PDAWAKECFG = 0x00006D00; //everything is power-up
  //todo power down unnecessary peripheral after this by writing 1 to SYSCON_PDAWAKECFG

  //got SYSCON_SYSPLLCTRL and SYSCON_MAINCLKSEL from
  //http://docs.lpcware.com/lpc800um/RegisterMaps/syscon/c-SystemPLLfunctionaldescription.html
  SYSCON_SYSPLLCLKSEL = 0x01; //crystal oscillator
  SYSCON_SYSOSCCTRL &= ~(1 << 0); //disable BYPASS because of using crystal oscillator
  SYSCON_SYSPLLCTRL = (4 << 0) | (0 << 5); //todo P and M dividers? have no idea what's that
  SYSCON_SYSPLLCLKUEN = 0x01; //update PLL clock source
  while (!(SYSCON_SYSPLLSTAT & 0x01))
    ; //wait for pll lock.
  SYSCON_MAINCLKSEL = 0x03; //use PLL as main clock
  SYSCON_MAINCLKUEN = 0x01; //inform core about clock update
}
//////////////////////////////////////////////////////////////////////////

void
SystemCoreClockUpdate (void) {

}
//////////////////////////////////////////////////////////////////////////
