#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include "lpc824.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define F_CPU 12000000UL
#define UNUSED_ARG(x) ((void)x)

uint16_t crc16(uint8_t *data, uint8_t len);

// MEGAGLOBAL VARIABLE. AHTUNG
// will be used in main function
volatile uint32_t SoftwareInterruptsFlag;

#define SINT_USART0_MB_TSX (1 << 0)
#define SINT_ADXL_X_UPDATED (1 << 1)
#define SINT_ADXL_Y_UPDATED (1 << 2)
#define SINT_ADXL_Z_UPDATED (1 << 3)

#define SetSoftwareInt(x) ((SoftwareInterruptsFlag |= x))
#define ClrSoftwareInt(x) ((SoftwareInterruptsFlag &= ~x))

typedef struct {
  int quot; /*!< Quotient */
  int rem;  /*!< Remainder */
} IDIV_RETURN_T;

typedef struct {
  unsigned quot; /*!< Quotient */
  unsigned rem;  /*!< Reminder */
} UIDIV_RETURN_T;

typedef struct {
  /*!< Signed integer division */
  int (*sidiv)(int numerator, int denominator);
  /*!< Unsigned integer division */
  unsigned (*uidiv)(unsigned numerator, unsigned denominator);
  /*!< Signed integer division with remainder */
  IDIV_RETURN_T (*sidivmod)(int numerator, int denominator);
  /*!< Unsigned integer division with remainder */
  UIDIV_RETURN_T (*uidivmod)(unsigned numerator, unsigned denominator);
} ROM_DIV_API_T;

typedef struct {
  const uint32_t reserved0; /*!< Reserved */
  const uint32_t reserved1; /*!< Reserved */
  const uint32_t reserved2; /*!< Reserved */
  const void *pPWRD;        /*!< Power API function table base address */
  const ROM_DIV_API_T
      *divApiBase;          /*!< Divider API function table base address */
  const void *pI2CD;        /*!< I2C driver routines functions table */
  const uint32_t reserved5; /*!< Reserved */
  const void *pSPID;        /*!< SPI driver API function table base address */
  const void *pADCD;        /*!< ADC driver API function table base address */
  const void *pUARTD;       /*!< USART driver API function table base address */
} LPC_ROM_API_T;
#define ROM_DRIVER_BASE (0x1FFF1FF8UL)
#define LPC_DIV_API \
  ((ROM_DIV_API_T *)((*(LPC_ROM_API_T **)(ROM_DRIVER_BASE))->divApiBase))

#endif  // COMMONS_H
