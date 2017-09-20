#include "adxl363.h"
#include "lpc824.h"
#include "commons.h"

#define SPI_CFG_ENABLE (1)
#define SPI_CFG_MASTER (1 << 2)
#define SPI_CFG_LSBF (1 << 3)

#define SPI_STAT_RXRDY (0x1)
#define SPI_STAT_TXRDY (0x2)
#define SPI_STAT_MSTIDLE (0x100)

#define SPI_TXDATCTL_SSEL_N(s) ((s) << 16)
#define SPI_TXDATCTL_EOT (1 << 20)
#define SPI_TXDATCTL_EOF (1 << 21)
#define SPI_TXDATCTL_RXIGNORE (1 << 22)
#define SPI_TXDATCTL_FLEN(l) ((l) << 24)

#define SPI_DLY_PREDELAY(d) ((d) << 0)
#define SPI_DLY_POSTDELAY(d) ((d) << 4)
#define SPI_DLY_FRAMEDELAY(d) ((d) << 8)
#define SPI_DLY_INTERDELAY(d) ((d) << 12)

//first 3 registers
#define ADXL363_SIGNATURE 0x00ad1df3

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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static uint16_t readRegisterSync(uint8_t reg);
static void writeRegisterSync(uint8_t reg, uint8_t val);
static adxl_range_t m_currentRange;

#define RXRDY (1 << 0)
#define TXRDY (1 << 1)
static inline void spi0RxEnable() { SPI0_INTENSET |= RXRDY;}
static inline void spi0RxDisable() { SPI0_INTENCLR |= RXRDY;}
static inline void spi0TxReadyEnable() { SPI0_INTENSET |= TXRDY;}
static inline void spi0TxReadyDisable() { SPI0_INTENSET |= TXRDY;}

static uint8_t rxBuffer[6] = {0}; //x, y, z
static uint8_t rxCurIx = 0;

static uint8_t cmdBuff[6] = {AR_XDATA_H, AR_XDATA_L,
                             AR_YDATA_H, AR_YDATA_L,
                             AR_ZDATA_H, AR_ZDATA_L};


static uint8_t cmdIx = 0;

//enabled only for x, y, z data.
void SPI0_IRQHandler(void) {
  static volatile uint8_t txCount = 0;

  uint32_t is = SPI0_INSTAT;

  if (is & RXRDY) {
    rxBuffer[rxCurIx] = SPI0_RXDAT;
    switch (rxCurIx) {
      case 1:
        SetSoftwareInt(SINT_ADXL_X_UPDATED);
        break;
      case 3:
        SetSoftwareInt(SINT_ADXL_Y_UPDATED);
        break;
      case 5:
        SetSoftwareInt(SINT_ADXL_Z_UPDATED);
        break;
    }
    if (++rxCurIx >= 6) rxCurIx = 0;
  }

  if (is & TXRDY) {
    if (txCount) { //end of transaction
      SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(7) |      //1 byte
                      SPI_TXDATCTL_EOT |          //end of transaction.
                      SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                      0x00;
      txCount = 0;
    } else { //begin of transaction
      SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(15) |     //2 bytes
                      SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                      SPI_TXDATCTL_RXIGNORE |
                      (uint16_t)((adxl_read_r << 8) | cmdBuff[cmdIx++]);
      if (cmdIx >= 6) cmdIx = 0;
      txCount = 1;
    }
  }
}
//////////////////////////////////////////////////////////////////////////

static inline void adxlConfigPins() {
  SWM_PINASSIGN3 = 0x0dffffff;  //SPI0_CLK -> PIO0_13
  SWM_PINASSIGN4 = 0xff000017 | //SPI0_MOSI -> PIO0_23
                   0xff001100 | //SPI0_MISO -> PIO0_17
                   0xff0c0000;  //SPI0_SSEL0 -> PIO0_12
}
//////////////////////////////////////////////////////////////////////////

void adxlReset(void) {
  static uint8_t init_data[] = {
    0xfa, 0x00, //set activity threshold to 250 mg 0x20, 0x21 LOW then HIGH half
    0x00,       //set activity time to 0  0x22
    0x96, 0x00, //set inactivity threshold to 150 mg 0x23, 0x24 LOW then HIGH half
    0x1e, 0x00, //set inactivity timer to 30 samples 0x25, 0x26 LOW then HIGH half
    0x00,       //disable activity/inactivity interrupts 0x27
    0x00, 0x00, //don't use FIFO, 0x28, 0x29
    0x00, 0x00, //no interrupts? map registers 0x2a, 0x2b
    0x13, //+-2g, HALF_BW, no EXT_SAMPLE, 100Hz, 0x2c
    0x12  //adc disabled, no ext clock, low noise, no wake-up,
    //no autosleep, measurement mode 0x2d
  };
  register int32_t i;
  m_currentRange = adxlr_2g;
  adxlConfigPins();
  SYSCON_PRESETCTRL &= ~(1 << 0); //reset SPI0
  SYSCON_SYSAHBCLKCTRL |= (1 << 11); //enable clock for SPI0
  SYSCON_PRESETCTRL |= (1 << 0); //take SPI0 out of reset
  SPI0_DLY = 0x00001009; //1 tick post and pre delay. 1 tick transaction delay.
  SPI0_DIV = 0x0018; //divider is 24. result is 0.5 MHz
  SPI0_CFG = SPI_CFG_ENABLE | SPI_CFG_MASTER ; //enable SPI0 master mode, CPHA = CPOL = 0

  writeRegisterSync(AR_SOFT_RESET, 0x52);
  for (i = 100; --i;); //wait
  for (i = 0x20; i <= 0x2d; ++i)
    writeRegisterSync(i, init_data[i-0x20]);
  for (i = 100; --i;); //wait

  cmdIx = rxCurIx = 0;
  NVIC_ISER0 |= (1 << 0); //enable SPI0 interrupt.

  spi0RxEnable();
  spi0TxReadyEnable();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//warning! use external disabling of interrups before call
uint16_t readRegisterSync(uint8_t reg) {
  uint32_t rx_data;
  while(~SPI0_STAT & SPI_STAT_TXRDY) ;
  SPI0_TXDATCTL = SPI_TXDATCTL_FLEN(15) |     //2 bytes
                  SPI_TXDATCTL_SSEL_N(0xe) |  //SSEL0 asserted
                  SPI_TXDATCTL_RXIGNORE |
                  (uint16_t)((adxl_read_r << 8) | reg);

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

//warning! use external disabling of interrups before call
void writeRegisterSync(uint8_t reg,
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

void adxlSetRange(adxl_range_t range) {
  uint8_t reg;
  spi0RxDisable();
  spi0TxReadyDisable();
  reg = (uint8_t) readRegisterSync(AR_FILTER_CTL);
  reg &= 0x3f;
  reg |= (range << 6);
  writeRegisterSync(AR_FILTER_CTL, reg);
  m_currentRange = range;

  for (reg = 0; reg < 6; ++reg)
    rxBuffer[reg] = 0;
  cmdIx = rxCurIx = 0;

  spi0RxEnable();
  spi0TxReadyEnable();
}
//////////////////////////////////////////////////////////////////////////

void adxlSetOdr(adxl_odr_t odr) {
  uint8_t reg;

  spi0RxDisable();
  spi0TxReadyDisable();

  reg = (uint8_t) readRegisterSync(AR_FILTER_CTL);
  reg &= 0xf8;
  reg |= odr;
  writeRegisterSync(AR_FILTER_CTL, reg);

  for (reg = 0; reg < 6; ++reg)
    rxBuffer[reg] = 0;
  cmdIx = rxCurIx = 0;

  spi0RxEnable();
  spi0TxReadyEnable();
}
//////////////////////////////////////////////////////////////////////////

int16_t adxl_X(void) {
  int16_t res = (uint16_t)rxBuffer[0] << 8;
  res |= rxBuffer[1];
  return res;
}
//////////////////////////////////////////////////////////////////////////

int16_t adxl_Y(void) {
  int16_t res = (uint16_t)rxBuffer[2] << 8;
  res |= rxBuffer[3];
  return res;
}
//////////////////////////////////////////////////////////////////////////

int16_t adxl_Z(void) {
  int16_t res = (uint16_t)rxBuffer[4] << 8;
  res |= rxBuffer[5];
  return res;
}
//////////////////////////////////////////////////////////////////////////

//uint32_t adxlDevSign() {
//  uint32_t res = 0x00000000;
//  res |= readRegisterSync(AR_DEVID_AD) << 16;
//  res |= readRegisterSync(AR_DEVID_MST) << 8;
//  res |= readRegisterSync(AR_DEVID);
//  return res;
//}
////////////////////////////////////////////////////////////////////////////

//uint8_t adxlCheckSign() {
//  return adxlDevSign() == ADXL363_SIGNATURE;
//}
////////////////////////////////////////////////////////////////////////////

//adxl_range_t adxlCurrentRange() {
//  return m_currentRange;
//}
