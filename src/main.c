#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "lpc824.h"
#include "lpc824_api.h"

void config_pins();

typedef enum adxl_consts {
  /*registers*/
  DEVID_AD = 0x00, //[7:0] DEVID_AD[7:0] 0xAD R
  DEVID_MST = 0x01, // [7:0] DEVID_MST[7:0] 0x1D R
  DEVID = 0x02, // [7:0] DEVID[7:0] 0xF3 R
  REVID=0x03 , // [7:0] REVID[7:0] 0x01 R
  XDATA=0x08 , // [7:0] XDATA[7:0] 0x00 R
  YDATA =0x09 , //[7:0] YDATA[7:0] 0x00 R
  ZDATA =0x0A , //[7:0] ZDATA[7:0] 0x00 R
  STATUS=0x0B , // [7:0]
  FIFO_ENTRIES_L=0x0C , // [7:0] FIFO_ENTRIES_L[7:0] 0x00 R
  FIFO_ENTRIES_H=0x0D , // [7:0] UNUSED FIFO_ENTRIES_H[1:0] 0x00 R
  XDATA_L=0x0E , // [7:0] XDATA_L[7:0] 0x00 R
  XDATA_H=0x0F , // [7:0] SX XDATA_H[3:0] 0x00 R
  YDATA_L=0x10 , // [7:0] YDATA_L[7:0] 0x00 R
  YDATA_H=0x11 , // , //[7:0] SX YDATA_H[3:0] 0x00 R
  ZDATA_L=0x12 , // [7:0] ZDATA_L[7:0] 0x00 R
  ZDATA_H=0x13 , // [7:0] SX ZDATA_H[3:0] 0x00 R
  TEMP_L=0x14 , // [7:0] TEMP_L[7:0] 0x00 R
  TEMP_H=0x15 , // [7:0] SX TEMP_H[3:0] 0x00 R
  ADC_DATA_L =0x16 , //[7:0] ADC_DATA_L[7:0] 0x00 R
  ADC_DATA_H=0x17 , // [7:0] SX ADC_DATA_H[2:0] 0x00 R
  SOFT_RESET=0x1F , // [7:0] SOFT_RESET[7:0] 0x00 W
  THRESH_ACT_L=0x20 , // [7:0] THRESH_ACT_L[7:0] 0x00 RW
  THRESH_ACT_H=0x21 , //[7:0] UNUSED THRESH_ACT_H[2:0] 0x00 RW
  TIME_ACT =0x22 , //[7:0] TIME_ACT[7:0] 0x00 RW
  THRESH_INACT_L=0x23 , // [7:0] THRESH_INACT_L[7:0] 0x00 RW
  THRESH_INACT_H =0x24 , //[7:0] UNUSED THRESH_INACT_H[2:0] 0x00 RW
  TIME_INACT_L=0x25 , // [7:0] TIME_INACT_L[7:0] 0x00 RW
  TIME_INACT_H=0x26 , // [7:0] TIME_INACT_H[7:0] 0x00 RW
  ACT_INACT_CTL=0x27 , //[7:0] UNUSED LINKLOOP INACT_REF INACT_EN ACT_REF ACT_EN 0x00 RW
  FIFO_CONTROL=0x28 , // [7:0] UNUSED AH FIFO_TEMP FIFO_MODE 0x00 RW
  FIFO_SAMPLES =0x29 , //[7:0] FIFO_SAMPLES[7:0] 0x80 RW
  INTMAP1=0x2A, // [7:0], // INT_LOW AWAKE INACT ACT FIFO_
  INTMAP2=0x2B , // [7:0] INT_LOW AWAKE INACT ACT FIFO_
  FILTER_CTL =0x2C, //[7:0] RANGE RES HALF_BW EXT_SAMPLE ODR 0x13 RW
  POWER_CTL=0x2D, // [7:0] ADC_EN EXT_CLK LOW_NOISE WAKEUP AUTOSLEEP MEASURE 0x00 RW
  SELF_TEST=0x2E, // [7:0] UNUSED ST 0x00 RW
  /*spi commands*/
  adxl_write_r = 0x0a,
  adxl_read_r  = 0x0b,
  adxl_read_fifo  = 0x0d
} adxl_consts_t;
//////////////////////////////////////////////////////////////////////////

uint16_t
send_adxl_command(uint8_t rw, uint8_t cmd, uint8_t val);

void wait(uint32_t timeout) {
  while (--timeout) ;
}

  /*
   * init adxl363
    1. Set activity and inactivity thresholds and timers.
      a. Write to Register 0x20 to Register 0x26.
      b. To minimize false positive motion triggers, set the
      TIME_ACT register greater than 1.
    2. Configure activity and inactivity functions.
      a. Write to Register 0x27.
    3. Configure FIFO.
      a. Write to Register 0x28 and Register 0x29.
    4. Map interrupts.
      a. Write to Register 0x2A and Register 0x2B.
    5. Configure general device settings.
      a. Write to Register 0x2C.
    6. Turn measurement on.
      a. Write to Register 0x2D.
    7. Wait 4/ODR for data to settle before reading the data
    registers.
  */
int
main(void) {
  uint8_t commands[] = {STATUS, DEVID, 0xff};
  uint8_t init_data[] = {
    0xfa, 0x00, //set activity threshold to 250 mg 0x20, 0x21 LOW then HIGH half
    0x00,       //set activity time to 0  0x22
    0x96, 0x00, //set inactivity threshold to 150 mg 0x23, 0x24 LOW then HIGH half
    0x1e, 0x00, //set inactivity timer to 30 samples 0x25, 0x26 LOW then HIGH half
    0x00,       //disable activity/inactivity interrupts 0x27
    0x00, 0x00, //don't use FIFO, 0x28, 0x29
    0x00, 0x00, //no interrupts? map registers 0x2a, 0x2b
    0b00010011, //+-2g, HALF_BW, no EXT_SAMPLE, 100Hz, 0x2c
    0b00010010  //adc disabled, no ext clock, low noise, no wake-up,
                //no autosleep, measurement mode 0x2d
  };

  volatile uint16_t adxl_data = 0;
  uint32_t i;

  SYSCON_PRESETCTRL &= ~(1 << 0); //reset SPI0
  SYSCON_SYSAHBCLKCTRL |= (1 << 11); //enable clock for SPI0
  SYSCON_PRESETCTRL |= (1 << 0); //take SPI0 out of reset

  NVIC_ISER0 |= (1 << 0); //enable SPI0 interrupt. why?
  SPI0_DLY = 0x00001009; //1 tick post and pre delay. 1 tick transaction delay.
  SPI0_DIV = 0x0018; //divider is 24. result is 0.5 MHz
  SPI0_CFG = SPI_CFG_ENABLE | SPI_CFG_MASTER ; //enable SPI0 master mode, CPHA = CPOL = 0

  while(1);

  adxl_data = send_adxl_command(adxl_write_r, SOFT_RESET, 0x52);
  wait(1000000);

  for (i = 0x20; i <= 0x2d; ++i) {
    send_adxl_command(adxl_write_r, i, init_data[i-0x20]);
  }
  wait(1000000);

  while (1) {
    for (i = 0; commands[i] != 0xff; ++i) {
      adxl_data = send_adxl_command(adxl_read_r, commands[i], 0x00);
    }
  }

  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
config_pins() {
  /*base config*/
  SWM_PINENABLE0 &= ~(1 << 5); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 6); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 7); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 8); // CLKIN on pin PIO0_1

  /*spi0 config*/
//  GPIO_DIR0 |= (1 << 0) | (1 << 2) | (1 << 3);
//  GPIO_DIR0 &= ~(1 << 1);

  SWM_PINASSIGN3 = 0x0d000000;  //SPI0_CLK -> PIO0_13
  SWM_PINASSIGN4 = 0x00000017 | //SPI0_MOSI -> PIO0_23
                   0x00001100 | //SPI0_MISO -> PIO0_17
                   0x000c0000;  //SPI0_SSEL0 -> PIO0_12

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

uint16_t
send_adxl_command(uint8_t rw,
                  uint8_t cmd,
                  uint8_t val) {
  uint32_t rx_data;
  uint16_t adxl_data;

  while(~SPI0_STAT & SPI_STAT_TXRDY) ;

  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(15) |     //2 bytes
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  (uint16_t)((rw << 8) | cmd);

  while(~SPI0_STAT & SPI_STAT_RXRDY) ;

  rx_data = SPI0_RXDAT;
  adxl_data = (uint16_t) (rx_data & 0x00ff);
  while(~SPI0_STAT & SPI_STAT_TXRDY) ;

  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(7) |      //1 byte
                  SPI_TXDATCTL_EOT |          //end of transaction.
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  val;
  while(~SPI0_STAT & SPI_STAT_RXRDY) ;

  rx_data = SPI0_RXDAT;
  adxl_data = (uint16_t) (rx_data & 0x00ff);

  while(~SPI0_STAT & SPI_STAT_MSTIDLE) ;
  return adxl_data;
}
