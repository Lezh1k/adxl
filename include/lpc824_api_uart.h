#ifndef LPC824_API_UART_H
#define LPC824_API_UART_H

#include <stdint.h>

typedef void *uart_handle_t ; // define TYPE for uart handle pointer
typedef uint32_t (*uart_callback_t)();

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
  uart_callback_t callback_func_pt; // callback function
} uart_param_t ;

typedef struct uart_config {
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

typedef struct uart_api { // index of all the uart driver functions
  uint32_t (*uart_get_mem_size)(void);
  uart_handle_t (*uart_setup)(uint32_t base_addr, uint8_t *ram);
  uint32_t (*uart_init)(uart_handle_t handle, uart_config_t *set);
  //--polling functions--//
  uint8_t (*uart_get_char)(uart_handle_t handle);
  void (*uart_put_char)(uart_handle_t handle, uint8_t data);
  uint32_t (*uart_get_line)(uart_handle_t handle, uart_param_t * param);
  uint32_t (*uart_put_line)(uart_handle_t handle, uart_param_t * param);
  //--interrupt functions--//
  void (*uart_isr)(uart_handle_t handle);
} uart_api_t ; // end of structure

#endif // LPC824_API_UART_H
