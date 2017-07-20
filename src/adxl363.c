#include "adxl363.h"
#include "lpc824.h"
#include "lpc824_api_spi.h"

typedef enum adxl_register {
  /*registers*/
  AR_DEVID_AD = 0x00,        //[7:0] DEVID_AD[7:0] 0xAD R
  AR_DEVID_MST = 0x01,       // [7:0] DEVID_MST[7:0] 0x1D R
  AR_DEVID = 0x02,           // [7:0] DEVID[7:0] 0xF3 R
  AR_REVID = 0x03,           // [7:0] REVID[7:0] 0x01 R
  AR_XDATA = 0x08,           // [7:0] XDATA[7:0] 0x00 R
  AR_YDATA = 0x09,           //[7:0] YDATA[7:0] 0x00 R
  AR_ZDATA = 0x0A,           //[7:0] ZDATA[7:0] 0x00 R
  AR_STATUS = 0x0B,          // [7:0]
  AR_FIFO_ENTRIES_L = 0x0C,  // [7:0] FIFO_ENTRIES_L[7:0] 0x00 R
  AR_FIFO_ENTRIES_H = 0x0D,  // [7:0] UNUSED FIFO_ENTRIES_H[1:0] 0x00 R
  AR_XDATA_L = 0x0E,         // [7:0] XDATA_L[7:0] 0x00 R
  AR_XDATA_H = 0x0F,         // [7:0] SX XDATA_H[3:0] 0x00 R
  AR_YDATA_L = 0x10,         // [7:0] YDATA_L[7:0] 0x00 R
  AR_YDATA_H = 0x11,         // , //[7:0] SX YDATA_H[3:0] 0x00 R
  AR_ZDATA_L = 0x12,         // [7:0] ZDATA_L[7:0] 0x00 R
  AR_ZDATA_H = 0x13,         // [7:0] SX ZDATA_H[3:0] 0x00 R
  AR_TEMP_L = 0x14,          // [7:0] TEMP_L[7:0] 0x00 R
  AR_TEMP_H = 0x15,          // [7:0] SX TEMP_H[3:0] 0x00 R
  AR_ADC_DATA_L = 0x16,      //[7:0] ADC_DATA_L[7:0] 0x00 R
  AR_ADC_DATA_H = 0x17,      // [7:0] SX ADC_DATA_H[2:0] 0x00 R
  AR_SOFT_RESET = 0x1F,      // [7:0] SOFT_RESET[7:0] 0x00 W
  AR_THRESH_ACT_L = 0x20,    // [7:0] THRESH_ACT_L[7:0] 0x00 RW
  AR_THRESH_ACT_H = 0x21,    //[7:0] UNUSED THRESH_ACT_H[2:0] 0x00 RW
  AR_TIME_ACT = 0x22,        //[7:0] TIME_ACT[7:0] 0x00 RW
  AR_THRESH_INACT_L = 0x23,  // [7:0] THRESH_INACT_L[7:0] 0x00 RW
  AR_THRESH_INACT_H = 0x24,  //[7:0] UNUSED THRESH_INACT_H[2:0] 0x00 RW
  AR_TIME_INACT_L = 0x25,    // [7:0] TIME_INACT_L[7:0] 0x00 RW
  AR_TIME_INACT_H = 0x26,    // [7:0] TIME_INACT_H[7:0] 0x00 RW
  AR_ACT_INACT_CTL =
  0x27,  //[7:0] UNUSED LINKLOOP INACT_REF INACT_EN ACT_REF ACT_EN 0x00 RW
  AR_FIFO_CONTROL = 0x28,  // [7:0] UNUSED AH FIFO_TEMP FIFO_MODE 0x00 RW
  AR_FIFO_SAMPLES = 0x29,  //[7:0] FIFO_SAMPLES[7:0] 0x80 RW
  AR_INTMAP1 = 0x2A,       // [7:0], // INT_LOW AWAKE INACT ACT FIFO_
  AR_INTMAP2 = 0x2B,       // [7:0] INT_LOW AWAKE INACT ACT FIFO_
  AR_FILTER_CTL = 0x2C,    //[7:0] RANGE RES HALF_BW EXT_SAMPLE ODR 0x13 RW
  AR_POWER_CTL =
  0x2D,  // [7:0] ADC_EN EXT_CLK LOW_NOISE WAKEUP AUTOSLEEP MEASURE 0x00 RW
  AR_SELF_TEST = 0x2E,  // [7:0] UNUSED ST 0x00 RW
} adxl_register_t;
//////////////////////////////////////////////////////////////////////////

typedef enum adxl_cmd {
  adxl_write_r = 0x0a,
  adxl_read_r = 0x0b,
  adxl_read_fifo = 0x0d
} adxl_cmd_t;
//////////////////////////////////////////////////////////////////////////

uint16_t read_register(uint8_t reg);
void write_register(uint8_t reg,
                    uint8_t val);
//////////////////////////////////////////////////////////////////////////

void
adxl_init(void) {
  static uint8_t init_data[] = {
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
  register int32_t i;

  /*spi0 config*/
  SWM_PINASSIGN3 = 0x0dffffff;  //SPI0_CLK -> PIO0_13
  SWM_PINASSIGN4 = 0xff000017 | //SPI0_MOSI -> PIO0_23
                   0xff001100 | //SPI0_MISO -> PIO0_17
                   0xff0c0000;  //SPI0_SSEL0 -> PIO0_12

  SYSCON_PRESETCTRL &= ~(1 << 0); //reset SPI0
  SYSCON_SYSAHBCLKCTRL |= (1 << 11); //enable clock for SPI0
  SYSCON_PRESETCTRL |= (1 << 0); //take SPI0 out of reset

  NVIC_ISER0 |= (1 << 0); //enable SPI0 interrupt. why?
  SPI0_DLY = 0x00001009; //1 tick post and pre delay. 1 tick transaction delay.
  SPI0_DIV = 0x0018; //divider is 24. result is 0.5 MHz
  SPI0_CFG = SPI_CFG_ENABLE | SPI_CFG_MASTER ; //enable SPI0 master mode, CPHA = CPOL = 0

  write_register(AR_SOFT_RESET, 0x52);
  for (i = 10000; --i;); //wait

  for (i = 0x20; i <= 0x2d; ++i) {
    write_register(i, init_data[i-0x20]);
  }

  for (i = 10000; --i;); //wait
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint16_t read_register(uint8_t reg) {
  uint32_t rx_data;
  while(~SPI0_STAT & SPI_STAT_TXRDY) ;
  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(15) |     //2 bytes
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  (uint16_t)((adxl_read_r << 8) | reg);

  while(~SPI0_STAT & SPI_STAT_RXRDY) ;

  rx_data = SPI0_RXDAT;
  while(~SPI0_STAT & SPI_STAT_TXRDY) ;

  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(7) |      //1 byte
                  SPI_TXDATCTL_EOT |          //end of transaction.
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  0x00;
  while(~SPI0_STAT & SPI_STAT_RXRDY) ;
  rx_data = SPI0_RXDAT;
  while(~SPI0_STAT & SPI_STAT_MSTIDLE) ;
  return (uint16_t)(rx_data & 0x0000ffff);
}
//////////////////////////////////////////////////////////////////////////

void
write_register(uint8_t reg,
               uint8_t val) {
  while(~SPI0_STAT & SPI_STAT_TXRDY) ;
  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(15) |     //2 bytes
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  SPI_TXDATCTL_RXIGNORE |
                  (uint16_t)((adxl_write_r << 8) | reg);

  while(~SPI0_STAT & SPI_STAT_TXRDY) ;

  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(7) |      //1 byte
                  SPI_TXDATCTL_EOT |          //end of transaction.
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  SPI_TXDATCTL_RXIGNORE |
                  val;
  while(~SPI0_STAT & SPI_STAT_MSTIDLE) ;
}
//////////////////////////////////////////////////////////////////////////

void
adxl_set_range(adxl_range_t range) {
  uint8_t reg = (uint8_t) read_register(AR_FILTER_CTL);
  reg &= 0x3f;
  reg |= (range << 6);
  write_register(AR_FILTER_CTL, reg);
}
//////////////////////////////////////////////////////////////////////////

void
adxl_set_odr(adxl_odr_t odr) {
  uint8_t reg = (uint8_t) read_register(AR_FILTER_CTL);
  reg &= 0xf8;
  reg |= odr;
  write_register(AR_FILTER_CTL, reg);
}
//////////////////////////////////////////////////////////////////////////

uint16_t
adxl_X(void) {
  uint16_t res = read_register(AR_XDATA_H) << 8;
  res |= read_register(AR_XDATA_L);
  return res & 0x0fff;
}
//////////////////////////////////////////////////////////////////////////

uint16_t
adxl_Y(void) {
  uint16_t res = read_register(AR_YDATA_H) << 8;
  res |= read_register(AR_YDATA_L);
  return res & 0x0fff;
}
//////////////////////////////////////////////////////////////////////////

uint16_t
adxl_Z(void) {
  uint16_t res = read_register(AR_ZDATA_H) << 8;
  res |= read_register(AR_ZDATA_L);
  return res & 0x0fff;
}
//////////////////////////////////////////////////////////////////////////

uint32_t
adxl_dev_sign() {
  uint32_t res = 0x00000000;
  res |= read_register(AR_DEVID_AD) << 16;
  res |= read_register(AR_DEVID_MST) << 8;
  res |= read_register(AR_DEVID);
  return res;
}
//////////////////////////////////////////////////////////////////////////
