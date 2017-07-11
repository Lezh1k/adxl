#include <stdint.h>
#include "lpc824.h"

//todo move api to api files. or api file :)

typedef struct _pwrd {
  void (*set_pll)(unsigned int cmd[], unsigned int resp[]);
  void (*set_power)(unsigned int cmd[], unsigned int resp[]);
} pwrd_api_t;
//////////////////////////////////////////////////////////////////////////

typedef struct {
  int32_t quot; /*!< Quotient */
  int32_t rem; /*!< Remainder */
} idiv_return_t;

typedef struct {
  uint32_t quot; /*!< Quotient */
  uint32_t rem; /*!< Reminder */
} uidiv_return_t;

typedef struct {
  int32_t (*sidiv)(int32_t numerator, int32_t denominator); /*!< Signed integer division */
  uint32_t (*uidiv)(uint32_t numerator, uint32_t denominator); /*!< Unsigned integer division */
  idiv_return_t (*sidivmod)(int32_t numerator, int32_t denominator); /*!< Signed integer division with remainder */
  uidiv_return_t (*uidivmod)(uint32_t numerator, uint32_t denominator); /*! <Unsigned integer division with remainder */
} rom_div_api_t;
//////////////////////////////////////////////////////////////////////////

typedef enum _i2c_err
{
  I2CE_OK=0, /**< enum value returned on Success */
  I2CE_ERROR,
  I2CE_ERR_I2C_BASE = 0x00060000,
  /*0x00060001*/ I2CE_ERR_I2C_NAK=I2CE_ERR_I2C_BASE+1,
  /*0x00060002*/ I2CE_ERR_I2C_BUFFER_OVERFLOW,
  /*0x00060003*/ I2CE_ERR_I2C_BYTE_COUNT_ERR,
  /*0x00060004*/ I2CE_ERR_I2C_LOSS_OF_ARBRITRATION,
  /*0x00060005*/ I2CE_ERR_I2C_SLAVE_NOT_ADDRESSED,
  /*0x00060006*/ I2CE_ERR_I2C_LOSS_OF_ARBRITRATION_NAK_BIT,
  /*0x00060007*/ I2CE_ERR_I2C_GENERAL_FAILURE,
  /*0x00060008*/ I2CE_ERR_I2C_REGS_SET_TO_DEFAULT
} i2c_err_t;

typedef struct i2c_R { // RESULTs struct--results are here when returned
  uint32_t n_bytes_sent ;
  uint32_t n_bytes_recd ;
} i2c_result ;

typedef void* i2c_handle_t;
typedef void (*i2c_callback_t) (uint32_t err_code, uint32_t n);

typedef enum i2c_mode {
  I2CM_IDLE,
  I2CM_MASTER_SEND,
  I2CM_MASTER_RECEIVE,
  I2CM_SLAVE_SEND,
  I2CM_SLAVE_RECEIVE
} i2c_mode_t ;

typedef struct i2c_A { //parameters passed to ROM function
  uint32_t num_bytes_send ;
  uint32_t num_bytes_rec ;
  uint8_t *buffer_ptr_send ;
  uint8_t *buffer_ptr_rec ;
  i2c_callback_t func_pt; // callback function pointer
  uint8_t stop_flag;
  uint8_t dummy[3] ; // required for word alignment
} i2c_param ;

typedef struct i2cd_api { // index of all the i2c driver functions
  void (*i2c_isr_handler) (i2c_handle_t* h_i2c) ; // ISR interrupt service request
  // MASTER functions ***
  i2c_err_t (*i2c_master_transmit_poll)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                        i2c_result* ptr );
  i2c_err_t (*i2c_master_receive_poll)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                       i2c_result* ptr );
  i2c_err_t (*i2c_master_tx_rx_poll)(i2c_handle_t* h_i2c,i2c_param* ptp,
                                     i2c_result* ptr ) ;
  i2c_err_t (*i2c_master_transmit_intr)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                        i2c_result* ptr ) ;
  i2c_err_t (*i2c_master_receive_intr)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                       i2c_result* ptr ) ;
  i2c_err_t (*i2c_master_tx_rx_intr)(i2c_handle_t* h_i2c, i2c_param* ptp, i2c_result*
                                     ptr ) ;
  // SLAVE functions ***
  i2c_err_t (*i2c_slave_receive_poll)(i2c_handle_t* h_i2c, i2c_param* ptp, i2c_result*
                                      ptr ) ;
  i2c_err_t (*i2c_slave_transmit_poll)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                       i2c_result* ptr ) ;
  i2c_err_t (*i2c_slave_receive_intr)(i2c_handle_t* h_i2c, i2c_param* ptp, i2c_result*
                                      ptr ) ;
  i2c_err_t (*i2c_slave_transmit_intr)(i2c_handle_t* h_i2c, i2c_param* ptp,
                                       i2c_result* ptr ) ;
  i2c_err_t (*i2c_set_slave_addr)(i2c_handle_t* h_i2c,
                                  uint32_t slave_addr_0_3, uint32_t slave_mask_0_3);
  // OTHER functions
  uint32_t (*i2c_get_mem_size)(void) ;
  i2c_handle_t* (*i2c_setup)(uint32_t i2c_base_addr, uint32_t *start_of_ram ) ;
  i2c_err_t (*i2c_set_bitrate)(i2c_handle_t* h_i2c, uint32_t P_clk_in_hz,
                               uint32_t bitrate_in_bps) ;
  uint32_t (*i2c_get_firmware_version)() ;
  i2c_mode_t (*i2c_get_status)(i2c_handle_t* h_i2c ) ;
} i2cd_api_t ;
//////////////////////////////////////////////////////////////////////////

typedef enum _spi_err
{
  SPI_ERR_SPI_BASE = 0x000E0000,
  /*0x000E0001*/ SPI_ERR_SPI_RXOVERRUN=SPI_ERR_SPI_BASE+1,
  /*0x000E0002*/ SPI_ERR_SPI_TXUNDERRUN,
  /*0x000E0003*/ SPI_ERR_SPI_SELNASSERT,
  /*0x000E0004*/ SPI_ERR_SPI_SELNDEASSERT,
  /*0x000E0005*/ SPI_ERR_SPI_CLKSTALL,
  /*0x000E0006*/ SPI_ERR_SPI_PARAM,
  /*0x000E0007*/ SPI_ERR_SPI_INVALID_LENGTH
} spi_err_t;

typedef struct {
  uint32_t delay;
  uint32_t divider;
  uint32_t config; // config register
  uint32_t error_en;//Bit0: OverrunEn, bit1: UnderrunEn,
}SPI_CONFIG_T;

typedef void* DMA_HANDLE_T;
typedef struct {
  uint32_t dma_txd_num; // SPI TX DMA channel number.
  uint32_t dma_rxd_num; // SPI RX DMA channel number. In order to do a SPI RX
  // DMA, a SPI TX DMA is also needed to generated SPI
  // clock.
  DMA_HANDLE_T hDMA;// DMA handle
} SPI_DMA_CFG_T;

typedef void (*SPI_CALLBK_T) (spi_err_t error_code, uint32_t num_transfer );
typedef void* SPI_HANDLE_T ;
typedef spi_err_t (*SPI_DMA_REQ_T) (SPI_HANDLE_T handle, SPI_DMA_CFG_T *dma_cfg);

typedef struct { // params passed to SPI driver function
  uint16_t *tx_buffer;// SPI TX buffer, needed in master and slave TX only
  //mode
  uint16_t *rx_buffer;// SPI RX buffer, needed in master RX only, TX and RX,
  //and slave RX mode,
  uint32_t size;// total number of SPI frames
  uint32_t fsize_sel;// data lenth of one transfer and SPI SSELx select in TXCTL
  uint32_t tx_rx_flag;
  uint32_t driver_mode;
  SPI_DMA_CFG_T *dma_cfg; // DMA configuration
  SPI_CALLBK_T cb; // callback function
  SPI_DMA_REQ_T dma_cb; // SPI DMA channel setup callback
} SPI_PARAM_T;

typedef struct { // index of all the SPI driver functions
  uint32_t (*spi_get_mem_size)(void);
  SPI_HANDLE_T (*spi_setup)(uint32_t base_addr, uint8_t *ram);
  void (*spi_init)(SPI_HANDLE_T handle, SPI_CONFIG_T *set);
  uint32_t (*spi_master_transfer)(SPI_HANDLE_T handle, SPI_PARAM_T * param);
  uint32_t (*spi_slave_transfer)(SPI_HANDLE_T handle, SPI_PARAM_T * param);
  //--interrupt functions--//
  void (*spi_isr)(SPI_HANDLE_T handle);
} SPID_API_T ;

//////////////////////////////////////////////////////////////////////////
typedef void *UART_HANDLE_T ; // define TYPE for uart handle pointer
typedef uint32_t (*UART_CALLBK_T)();

typedef struct uart_A { // parms passed to uart driver function
  uint8_t * buffer ; // The pointer of buffer.
  // For uart_get_line function, buffer for receiving data.
  // For uart_put_line function, buffer for transmitting data.
  uint32_t size; // [IN] The size of buffer.
  //[OUT] The number of bytes transmitted/received.
  uint16_t transfer_mode ;
  // 0x00: For uart_get_line function, transfer without
  // termination.
  // For uart_put_line function, transfer without termination.
  // 0x01: For uart_get_line function, stop transfer when
  // <CR><LF> are received.
  // For uart_put_line function, transfer is stopped after
  // reaching \0. <CR><LF> characters are sent out after that.
  // 0x02: For uart_get_line function, stop transfer when <LF>
  // is received.
  // For uart_put_line function, transfer is stopped after
  // reaching \0. A <LF> character is sent out after that.
  //0x03: For uart_get_line function, RESERVED.
  // For uart_put_line function, transfer is stopped after
  // reaching \0.
  uint16_t driver_mode;
  //0x00: Polling mode, function is blocked until transfer is
  // finished.
  // 0x01: Intr mode, function exit immediately, callback function
  // is invoked when transfer is finished.
  //0x02: RESERVED
  UART_CALLBK_T callback_func_pt; // callback function
} UART_PARAM_T ;

typedef struct UART_CONFIG {
  uint32_t sys_clk_in_hz; // main clock/UARTCLKDIV in Hz
  uint32_t baudrate_in_hz; // Baudrate in hz
  uint8_t config; //bit 1:0
  // 00: 7 bits length, 01: 8 bits lenght, others: reserved
  //bit3:2
  // 00: No Parity, 01: reserved, 10: Even, 11: Odd
  //bit4
  // 0: 1 Stop bit, 1: 2 Stop bits
  uint8_t sync_mod; //bit0: 0(Async mode), 1(Sync mode)
  //bit1: 0(Un_RXD is sampled on the falling edge of SCLK)
  // 1(Un_RXD is sampled on the rising edge of SCLK)
  //bit2: 0(Start and stop bits are transmitted as in asynchronous  mode)
  // 1(Start and stop bits are not transmitted)
  //bit3: 0(the UART is a slave on Sync mode)
  // 1(the UART is a master on Sync mode)
  uint16_t error_en; //Bit0: OverrunEn, bit1: UnderrunEn, bit2: FrameErrEn,
  // bit3: ParityErrEn, bit4: RxNoiseEn
} uart_config_t;

typedef struct UARTD_API { // index of all the uart driver functions
  uint32_t (*uart_get_mem_size)(void);
  UART_HANDLE_T (*uart_setup)(uint32_t base_addr, uint8_t *ram);
  uint32_t (*uart_init)(UART_HANDLE_T handle, uart_config_t *set);
  //--polling functions--//
  uint8_t (*uart_get_char)(UART_HANDLE_T handle);
  void (*uart_put_char)(UART_HANDLE_T handle, uint8_t data);
  uint32_t (*uart_get_line)(UART_HANDLE_T handle, UART_PARAM_T * param);
  uint32_t (*uart_put_line)(UART_HANDLE_T handle, UART_PARAM_T * param);
  //--interrupt functions--//
  void (*uart_isr)(UART_HANDLE_T handle);
} UARTD_API_T ; // end of structure

//////////////////////////////////////////////////////////////////////////

typedef struct _lpc_rom_api {
  const uint32_t reserved0; /*!< Reserved */
  const uint32_t reserved1; /*!< Reserved */
  const uint32_t reserved2; /*!< Reserved */
  const pwrd_api_t *pPWRD; /*!< Power API function table base address */
  const rom_div_api_t *divApiBase; /*!< Divider API function table base address */
  const i2cd_api_t *pI2CD;/*!< I2C driver routines functions table */
  const uint32_t reserved5; /*!< Reserved */
  const SPID_API_T *pSPID; /*!< SPI driver API function table base address */
  const void * pADCD; /*!< ADC driver API function table base address */
  const UARTD_API_T *pUARTD; /*!< USART driver API function table base address */
} lpc_rom_api_t;

#define ROM_DRIVER_BASE (0x1FFF1FF8UL)
#define LPC_PWRD_API ((pwrd_api_t *) ((*(lpc_rom_api_t **) (ROM_DRIVER_BASE))->pPWRD))
//rom_div_api_t const *pROMDiv = LPC_ROM_API->divApiBase;
#define ROM_DRIVER_BASE (0x1FFF1FF8UL)
#define ROM_DRIVER_BASE (0x1FFF1FF8UL)
#define LPC_I2CD_API ((I2CD_API_T *) ((*(ROM_API_T * *) (ROM_DRIVER_BASE))->pI2CD))
#define ROM_DRIVER_BASE (0x1FFF1FF8UL)
#define LPC_UART_API ((UARTD_API_T *) ((*(ROM_API_T * *) (ROM_DRIVER_BASE))->pUARTD))

int
main(void) {
  volatile int32_t i;
  volatile int32_t j;

  while (1) {
    PIO0_0 = 1;
    for (i = 0; i < 100000; ++i) {
      for (j = 0; j < 100000; ++j) {
        /*do nothing*/
      }
    }
    PIO0_0 = 0;
    for (i = 0; i < 100000; ++i) {
      for (j = 0; j < 100000; ++j) {
        /*do nothing*/
      }
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
SystemInit (void) {
  SWM_PINENABLE0 &= ~(1 << 6); // XTALIN on pin PIO0_8
  SWM_PINENABLE0 &= ~(1 << 7); // XTALOUT on pin PIO0_9
  SWM_PINENABLE0 &= ~(1 << 8); // RESET on pin PIO0_5
  SWM_PINENABLE0 &= ~(1 << 9); // CLKIN on pin PIO0_1

  SYSCON_PDAWAKECFG = 0x00006D00; //everything is power-up
  //todo power down unnecessary peripheral after this by writing 1 to SYSCON_PDAWAKECFG

  //got SYSCON_SYSPLLCTRL and SYSCON_MAINCLKSEL from
  //http://docs.lpcware.com/lpc800um/RegisterMaps/syscon/c-SystemPLLfunctionaldescription.html
  SYSCON_SYSPLLCLKSEL = 0x01; //crystal oscillator
  SYSCON_SYSOSCCTRL &= ~(1 << 0); //disable BYPASS because of using crystal oscillator
  SYSCON_SYSPLLCTRL = (4 << 0) | (0 << 5); //todo P and M dividers? have no idea what's that
  SYSCON_SYSPLLCLKUEN = 0x01; //update PLL clock source
  while (!(SYSCON_SYSPLLSTAT & 0x01))
    ; //wait for pll lock.
  SYSCON_MAINCLKSEL = 0x03; //use PLL as main clock
  SYSCON_MAINCLKUEN = 0x01; //inform core about clock update
}
//////////////////////////////////////////////////////////////////////////

void
SystemCoreClockUpdate (void) {

}
//////////////////////////////////////////////////////////////////////////
