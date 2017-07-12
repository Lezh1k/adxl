#include <stdint.h>
#include "lpc824.h"
#include "lpc824_api.h"


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
//  //todo power down unnecessary peripheral after this by writing 1 to SYSCON_PDAWAKECFG

//  //got SYSCON_SYSPLLCTRL and SYSCON_MAINCLKSEL from
//  //http://docs.lpcware.com/lpc800um/RegisterMaps/syscon/c-SystemPLLfunctionaldescription.html
//  SYSCON_SYSPLLCLKSEL = 0x01; //crystal oscillator
//  SYSCON_SYSOSCCTRL &= ~(1 << 0); //disable BYPASS because of using crystal oscillator
//  SYSCON_SYSPLLCTRL = (4 << 0) | (0 << 5); //todo P and M dividers? have no idea what's that
//  SYSCON_SYSPLLCLKUEN = 0x01; //update PLL clock source
//  while (!(SYSCON_SYSPLLSTAT & 0x01))
//    ; //wait for pll lock.
//  SYSCON_MAINCLKSEL = 0x03; //use PLL as main clock
//  SYSCON_MAINCLKUEN = 0x01; //inform core about clock update
}
//////////////////////////////////////////////////////////////////////////

void
SystemCoreClockUpdate (void) {

}
//////////////////////////////////////////////////////////////////////////
