#include <stdint.h>
#include "lpc824.h"
#include "lpc824_api.h"

void config_pins();

int
main(void) {
  lpc_rom_api_t* api_lpc_rom = (lpc_rom_api_t*)(ROM_DRIVER_BASE);
  spi_api_t* api_spi = api_lpc_rom->pSPID;
  spi_config_t spi_cfg = {0};


  config_pins();

  while (1) {
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

  /*spi0 config*/
  SWM_PINASSIGN3 |= 0x03000000; //SPI0_CLK -> PIN3
  SWM_PINASSIGN4 |= 0x00000001; //SPI0_MOSI -> PIN1
  SWM_PINASSIGN4 |= 0x00000200; //SPI0_MISO -> PIN2
  SWM_PINASSIGN4 |= 0x00040000; //SPI0_SSEL0 -> PIN4

  /*uart0 config*/
}

void
SystemInit (void) {
  SYSCON_PDRUNCFG &= ~(1 << 7); // Power up the PLL.

  SYSCON_SYSPLLCLKSEL = 0; // select internal RC oscillator
  SYSCON_SYSPLLCLKUEN = 1; // inform PLL of update

  SYSCON_SYSPLLCTRL = (4 << 0) | (0 << 5); // set divisors/multipliers
  SYSCON_SYSPLLCLKUEN = 1; // inform PLL of update

  SYSCON_MAINCLKSEL = 3; // Use PLL as main clock
  SYSCON_MAINCLKUEN = 1; // Inform core of clock update

  SYSCON_CLKOUTSEL = 3; // select main clock as clock out source
  SYSCON_CLKOUTDIV = 1; // divide down so easier to measure
  SYSCON_CLKOUTUEN = 1; // update clockout source

  SWM_PINASSIGN11 = (23 << 16); // route clock out on pin 1 (GPIO0_23)
}
//////////////////////////////////////////////////////////////////////////
