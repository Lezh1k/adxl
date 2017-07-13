#include <stdint.h>
#include "lpc824.h"
#include "lpc824_api.h"

void
soft_wait(int32_t tt) {
  while (--tt) {
    asm("nop");
  }
}

int
main(void) {
  GPIO_DIR0 |= (1 << 19);
  while (1) {
    GPIO_B19 = 1;
    soft_wait(100000);
    GPIO_B19 = 0;
    soft_wait(100000);
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
SystemInit (void) {
  SYSCON_PDRUNCFG &= ~(1 << 7); // Power up the PLL.
  SYSCON_SYSPLLCLKSEL = 0; // select internal RC oscillator
  SYSCON_SYSPLLCLKUEN = 1; // inform PLL of update
  SYSCON_SYSPLLCTRL = (4 << 0) | (0 << 5); // set divisors/multipliers
  SYSCON_SYSPLLCLKUEN = 1; // inform PLL of update
  SYSCON_MAINCLKSEL = 3; // Use PLL as main clock
  SYSCON_MAINCLKUEN = 1; // Inform core of clock update

  SWM_PINENABLE0 &= ~(1 << 6); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 7); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 8); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 9); // CLKIN on pin PIO0_1

  SYSCON_CLKOUTSEL = 3; // select main clock as clock out source
  SYSCON_CLKOUTDIV = 100; // divide down so easier to measure
  SYSCON_CLKOUTUEN = 1; // update clockout source
  SWM_PINASSIGN11 = (23 << 16); // route clock out on pin 1 (GPIO0_23)
}
//////////////////////////////////////////////////////////////////////////
