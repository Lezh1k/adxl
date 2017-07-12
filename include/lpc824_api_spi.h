#ifndef LPC824_API_SPI_H
#define LPC824_API_SPI_H

#include <stdint.h>
#include "lpc824_api_common.h"

typedef struct spi_config {
  uint32_t delay;
  uint32_t divider;
  uint32_t config; // config register
  uint32_t error_en;//Bit0: OverrunEn, bit1: UnderrunEn,
} spi_config_t;

typedef struct spi_dma_cfg {
  uint32_t dma_txd_num; // SPI TX DMA channel number.
  uint32_t dma_rxd_num; // SPI RX DMA channel number. In order to do a SPI RX
  // DMA, a SPI TX DMA is also needed to generated SPI
  // clock.
  dma_handle_t hDMA;// DMA handle
} spi_dma_cfg_t;

typedef void (*spi_callback_t) (error_code_t error_code, uint32_t num_transfer );
typedef void* spi_handle_t ;
typedef error_code_t (*spi_dma_req_t) (spi_handle_t handle, spi_dma_cfg_t *dma_cfg);

typedef struct spi_param { // params passed to SPI driver function
  uint16_t *tx_buffer;// SPI TX buffer, needed in master and slave TX only
  //mode
  uint16_t *rx_buffer;// SPI RX buffer, needed in master RX only, TX and RX,
  //and slave RX mode,
  uint32_t size;// total number of SPI frames
  uint32_t fsize_sel;// data lenth of one transfer and SPI SSELx select in TXCTL
  uint32_t tx_rx_flag;
  uint32_t driver_mode;
  spi_dma_cfg_t *dma_cfg; // DMA configuration
  spi_callback_t cb; // callback function
  spi_dma_req_t dma_cb; // SPI DMA channel setup callback
} spi_param_t;

typedef struct spi_api { // index of all the SPI driver functions
  uint32_t (*spi_get_mem_size)(void);
  spi_handle_t (*spi_setup)(uint32_t base_addr, uint8_t *ram);
  void (*spi_init)(spi_handle_t handle, spi_config_t *set);
  uint32_t (*spi_master_transfer)(spi_handle_t handle, spi_param_t * param);
  uint32_t (*spi_slave_transfer)(spi_handle_t handle, spi_param_t * param);
  //--interrupt functions--//
  void (*spi_isr)(spi_handle_t handle);
} spi_api_t ;

#endif // LPC824_API_SPI_H
