#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
//#include <math.h>

#include "lpc824.h"
#include "lpc824_api.h"
#include "adxl363.h"

inline void config_pins();

int
main(void) {

  int16_t x, y, z;
  uint32_t adxl_sign;
//  float ax, ay, az;
  adxl_reset();

  while (1) {
    x = adxl_X();
    y = adxl_Y();
    z = adxl_Z();
//    ax = asinf(x*1000.0f);
//    ay = asinf(y*1000.0f);
//    az = asinf(z*1000.0f);
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

  /*spi0 config / adxl*/
  SWM_PINASSIGN3 = 0x0dffffff;  //SPI0_CLK -> PIO0_13
  SWM_PINASSIGN4 = 0xff000017 | //SPI0_MOSI -> PIO0_23
                   0xff001100 | //SPI0_MISO -> PIO0_17
                   0xff0c0000;  //SPI0_SSEL0 -> PIO0_12

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
