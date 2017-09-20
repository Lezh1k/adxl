#ifndef ADXL363_H
#define ADXL363_H

#include <stdint.h>

typedef enum adxl_range {
  adxlr_2g = 0x00,
  adxlr_4g = 0x01,
  adxlr_8g = 0x02
} adxl_range_t;
//////////////////////////////////////////////////////////////////////////

typedef enum adxl_odr {
  odr_12_5 = 0x00,
  odr_25 = 0x01,
  odr_50 = 0x02,
  odr_100 = 0x03,
  odr_200 = 0x04,
  odr_400 = 0x07
} adxl_odr_t;
//////////////////////////////////////////////////////////////////////////

// todo make API
void adxlReset(void);
void adxlSetRange(adxl_range_t range);
void adxlSetOdr(adxl_odr_t odr);

int16_t adxl_X(void);
int16_t adxl_Y(void);
int16_t adxl_Z(void);
// uint32_t adxlDevSign(void);
// uint8_t adxlCheckSign(void);
// adxl_range_t adxlCurrentRange(void);

#endif  // ADXL363_H
