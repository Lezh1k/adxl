#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "lpc824.h"
#include "lpc824_api.h"
#include "adxl363.h"

void config_pins();

int
main(void) {

  uint16_t x, y, z;
  uint32_t adxl_sign;
  adxl_init();


  while (1) {
    x = adxl_X();
    y = adxl_Y();
    z = adxl_Z();

    //todo send x, y, z
    adxl_sign = adxl_dev_sign();
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

  /*uart0 config*/
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
