#ifndef LPC824_API_H
#define LPC824_API_H

#include <stdint.h>
#include <lpc824.h>

#include "lpc824_api_common.h"
#include "lpc824_api_power.h"
#include "lpc824_api_i2c.h"
#include "lpc824_api_spi.h"
#include "lpc824_api_uart.h"
#include "lpc824_api_div.h"

typedef struct _lpc_rom_api {
  const uint32_t reserved0; /*!< Reserved */
  const uint32_t reserved1; /*!< Reserved */
  const uint32_t reserved2; /*!< Reserved */
  const pwrd_api_t *pPWRD; /*!< Power API function table base address */
  const rom_div_api_t *divApiBase; /*!< Divider API function table base address */
  const i2cd_api_t *pI2CD;/*!< I2C driver routines functions table */
  const uint32_t reserved5; /*!< Reserved */
  const spi_api_t *pSPID; /*!< SPI driver API function table base address */
  //todo implement ADC API
  const void * pADCD; /*!< ADC driver API function table base address */
  const uart_api_t *pUARTD; /*!< USART driver API function table base address */
} lpc_rom_api_t;

#define ROM_DRIVER_BASE (0x1FFF1FF8UL)

#define LPC_PWRD_API ((pwrd_api_t *)  ((*(lpc_rom_api_t **) (ROM_DRIVER_BASE))->pPWRD))
#define LPC_I2CD_API ((i2c_api_t  *)  ((*(lpc_rom_api_t **) (ROM_DRIVER_BASE))->pI2CD))
#define LPC_UART_API ((uart_api_t *)  ((*(lpc_rom_api_t **) (ROM_DRIVER_BASE))->pUARTD))
#define LPC_SPI_API  ((spi_api_t  *)  ((*(lpc_rom_api_t **) (ROM_DRIVER_BASE))->pSPID))

#endif // LPC824_API_H
