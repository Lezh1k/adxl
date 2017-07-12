#ifndef LPC824_API_I2C_H
#define LPC824_API_I2C_H

#include <stdint.h>
#include "lpc824_api_common.h"

typedef void* i2c_handle_t;
typedef void (*i2c_callback_t) (uint32_t err_code, uint32_t n);

typedef struct _i2c_result { // RESULTs struct--results are here when returned
  uint32_t n_bytes_sent ;
  uint32_t n_bytes_recd ;
} i2c_result_t ;

typedef enum i2c_mode {
  I2CM_IDLE,
  I2CM_MASTER_SEND,
  I2CM_MASTER_RECEIVE,
  I2CM_SLAVE_SEND,
  I2CM_SLAVE_RECEIVE
} i2c_mode_t ;

typedef struct _i2c_param { //parameters passed to ROM function
  uint32_t num_bytes_send ;
  uint32_t num_bytes_rec ;
  uint8_t *buffer_ptr_send ;
  uint8_t *buffer_ptr_rec ;
  i2c_callback_t func_pt; // callback function pointer
  uint8_t stop_flag;
  uint8_t dummy[3] ; // required for word alignment
} i2c_param_t ;

typedef struct i2cd_api { // index of all the i2c driver functions
  void (*i2c_isr_handler) (i2c_handle_t* h_i2c) ; // ISR interrupt service request
  // MASTER functions ***
  error_code_t (*i2c_master_transmit_poll)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                           i2c_result_t* ptr );
  error_code_t (*i2c_master_receive_poll)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                          i2c_result_t* ptr );
  error_code_t (*i2c_master_tx_rx_poll)(i2c_handle_t* h_i2c,i2c_param_t* ptp,
                                        i2c_result_t* ptr ) ;
  error_code_t (*i2c_master_transmit_intr)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                           i2c_result_t* ptr ) ;
  error_code_t (*i2c_master_receive_intr)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                          i2c_result_t* ptr ) ;

  error_code_t (*i2c_master_tx_rx_intr)(i2c_handle_t* h_i2c,
                                        i2c_param_t* ptp,
                                        i2c_result_t* ptr ) ;
  // SLAVE functions ***
  error_code_t (*i2c_slave_receive_poll)(i2c_handle_t* h_i2c, i2c_param_t* ptp, i2c_result_t*
                                         ptr ) ;
  error_code_t (*i2c_slave_transmit_poll)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                          i2c_result_t* ptr ) ;
  error_code_t (*i2c_slave_receive_intr)(i2c_handle_t* h_i2c, i2c_param_t* ptp, i2c_result_t*
                                         ptr ) ;
  error_code_t (*i2c_slave_transmit_intr)(i2c_handle_t* h_i2c, i2c_param_t* ptp,
                                          i2c_result_t* ptr ) ;
  error_code_t (*i2c_set_slave_addr)(i2c_handle_t* h_i2c,
                                     uint32_t slave_addr_0_3, uint32_t slave_mask_0_3);
  // OTHER functions
  uint32_t (*i2c_get_mem_size)(void) ;
  i2c_handle_t* (*i2c_setup)(uint32_t i2c_base_addr, uint32_t *start_of_ram ) ;
  error_code_t (*i2c_set_bitrate)(i2c_handle_t* h_i2c, uint32_t P_clk_in_hz,
                                  uint32_t bitrate_in_bps) ;
  uint32_t (*i2c_get_firmware_version)() ;
  i2c_mode_t (*i2c_get_status)(i2c_handle_t* h_i2c ) ;
} i2cd_api_t ;


#endif // LPC824_API_I2C_H
