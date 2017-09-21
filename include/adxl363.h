#ifndef ADXL363_H
#define ADXL363_H

#include <stdint.h>

typedef enum adxl_range {
  adxlr_2g = 0x00,
  adxlr_4g = 0x01,
  adxlr_8g = 0x02
} adxl_range_t;
//////////////////////////////////////////////////////////////////////////

/*
 * Output Data Rate. Selects the ODR and configures internal filters to a
 * bandwidth of 1/2 or 1/4 the selected ODR, depending on the HALF_BW bit
 * setting
 */
typedef enum adxl_odr {
  adxl_odr_12_5 = 0x00,  // 12.5Hz
  adxl_odr_25 = 0x01,    // 25Hz
  adxl_odr_50 = 0x02,    // 50Hz
  adxl_odr_100 = 0x03,   // 100Hz
  adxl_odr_200 = 0x04,   // 200Hz
  adxl_odr_400 = 0x07    // 400Hz
} adxl_odr_t;

/*
 * Halved Bandwidth. Additional information is provided in the Antialiasing
 * section. 0x1 1 = the bandwidth of the antialiasing filters is set to 1/4 the
 * output data rate (ODR) for more conservative filtering. 0 = the bandwidth of
 * the filters is set to 1/2 the ODR for a wider bandwidth
 */
typedef enum adxl_half_bandwidth {
  adxl_bw_0 = 0,
  adxl_bw_1 = 1
} adxl_half_bandwidth_t;
//////////////////////////////////////////////////////////////////////////

void adxlReset(void);
void adxlSetRange(adxl_range_t range);
void adxlSetOdr(adxl_odr_t odr);
void adxlSetBandwidth(adxl_half_bandwidth_t bw);

int16_t adxl_X(void);
int16_t adxl_Y(void);
int16_t adxl_Z(void);

adxl_range_t adxlRange(void);
adxl_odr_t adxlOdr(void);
adxl_half_bandwidth_t adxlBW(void);

#endif  // ADXL363_H
