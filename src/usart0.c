#include "usart0.h"
#include "lpc824.h"
#include "commons.h"
#include "modbus_rtu_client.h"

#define UART_STAT_RXRDY (0x1 << 0)
#define UART_STAT_TXRDY (0x1 << 2)
#define UART_STAT_TXIDLE (0x1 << 3)

#define USART0_BAUD_RATE 19200UL
#define BOD_TICK_COUNT (F_CPU / USART0_BAUD_RATE)
#define HALF_BOD_TICK_COUNT (BOD_TICK_COUNT / 2)

static inline void startMrtTimer0(uint32_t val) {
  MRT_INTVAL0 = val;
}

static inline void startMrtTimer0Imm(uint32_t val) {
  MRT_INTVAL0 = val | (1 << 31);
}

static inline void stopMrtTimer0() {
  MRT_INTVAL0 = 0;
}

static inline void stopMrtTimer0Imm() {
  MRT_INTVAL0 = (1 << 31);
}
//////////////////////////////////////////////////////////////////////////

static inline void enableRxReady() { USART0_INTENSET |= (1 << 0); }
static inline void enableTxReady() { USART0_INTENSET |= (1 << 2); }
static inline void enableTxIdle()  { USART0_INTENSET |= (1 << 3); }

static inline void disableRxReady() { USART0_INTENCLR |= (1 << 0); }
static inline void disableTxReady() { USART0_INTENCLR |= (1 << 2); }
static inline void disableTxIdle()  { USART0_INTENCLR |= (1 << 3); }

//////////////////////////////////////////////////////////////////////////

static void configMrt0();
static void rxReady();
static void txIdle();
static void txReady();

void UART0_IRQHandler(void) {  
  register uint32_t intStatus = USART0_INTSTAT;
  if (intStatus & (1 << 0))
    rxReady();
  if (intStatus & (1 << 2))
    txReady();
  if (intStatus & (1 << 3))
    txIdle();
}
//////////////////////////////////////////////////////////////////////////

#define RECV_BUFF_LEN 257
static uint8_t  recvBuff[RECV_BUFF_LEN] = {0};
static uint8_t  mbRecvBuff[RECV_BUFF_LEN] = {0};
static volatile uint16_t recvIx = 0;
static volatile uint16_t intRecvIx = 0;
static volatile uint8_t halfSymbolIdleCount = 0;

void MRT_IRQHandler(void) {

  MRT_STAT0 |= (1 << 0); //clear interrupt request
  if (halfSymbolIdleCount++ < 35) return; //todo check this interval. should be 7-8 . works with 35. why?
  disableRxReady();
  stopMrtTimer0Imm();  
  recvIx = 0;
  SetSoftwareInt(SINT_USART0_MB_TSX);
}
//////////////////////////////////////////////////////////////////////////

void usart0MbTsxHandle() {  
  mb_handle_request(recvBuff, intRecvIx); //todo check result
  halfSymbolIdleCount = 0;
  intRecvIx = 0;
  enableRxReady();
}
////////////////////////////////////////////////////////////////////////////

void rxReady() {
  stopMrtTimer0Imm();  
  recvBuff[recvIx++] = USART0_RXDAT;
  intRecvIx = recvIx;
  halfSymbolIdleCount = 0;
  startMrtTimer0(HALF_BOD_TICK_COUNT);  
  if (recvIx >= RECV_BUFF_LEN) {
    recvIx = 0;
    stopMrtTimer0Imm();
    return;
  }
}
//////////////////////////////////////////////////////////////////////////

void txReady() {
  //ready to send
}
//////////////////////////////////////////////////////////////////////////

void txIdle() {
  //byte sent
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void usart0Init() {
  SWM_PINASSIGN0 = (0x0e << 0)  | //U0_TXD -> PIO0_14
                   (0x0b << 8)  | //U0_RXD -> PIO0_11
                   (0x0f << 16) | //U0_RTS -> PIO0_15 . RTS should be used as DE.
                   (0xff << 24);  //disable U0_DTS

  SYSCON_PRESETCTRL &= ~(1 << 3); //reset USART0
  SYSCON_SYSAHBCLKCTRL |= (1 << 14); //enable clock for USART0
  SYSCON_PRESETCTRL |= (1 << 3); //take USART0 out of reset
  NVIC_ISER0 |= (1 << 3); //enable USART0 IRQ

  SYSCON_UARTCLKDIV = 0x01; //uart clock divider is 1. result 12 MHz
  USART0_BRG = F_CPU / (USART0_BAUD_RATE * 16); //got from documentation.

  USART0_CFG = (1 << 0) ;     //enable usart0
  USART0_CFG |= (0x01 << 2);  //8 bit Data length
  USART0_CFG &= ~(0x03 << 4); //no parity
  USART0_CFG &= ~(1 << 6);    //1 stop bit
  USART0_CFG &= ~(1 << 11);   //asynchronous mode is select

  GPIO_DIR0  |= (1 << 10); //PIO0_10 to output
  USART0_CFG |= (1 << 18); //Output Enable Turnaround time enable for RS-485 operation
  USART0_CFG |= (1 << 20); //output enable select
  USART0_CFG |= (1 << 21); //output enable polarity

  GPIO_B10 = 0; //RE always should be 0. DON'T REMOVE!!!!!!!!!!

  enableRxReady();
//  enableTxIdle();
//  enableTxReady();

  configMrt0();
}
//////////////////////////////////////////////////////////////////////////

void usart0SendSync(uint8_t sb) {
  while(~USART0_STAT & UART_STAT_TXRDY);
  USART0_TXDAT = sb;
  while(~USART0_STAT & UART_STAT_TXIDLE);
}
//////////////////////////////////////////////////////////////////////////

uint8_t usart0RecvSync() {
  uint8_t sb = 0;
  while(~USART0_STAT & UART_STAT_RXRDY);
  sb = USART0_RXDAT;
  while(~USART0_STAT & UART_STAT_TXIDLE);
  return sb;
}
//////////////////////////////////////////////////////////////////////////

void configMrt0() {
  SYSCON_PRESETCTRL &= ~(1 << 7); //reset MRT
  SYSCON_SYSAHBCLKCTRL |= (1 << 10); //enable clock for MRT
  SYSCON_PRESETCTRL |= (1 << 7); //take MRT out of reset
  NVIC_ISER0 |= (1 << 10); //enable  NVIC MRT interrupt
  MRT_CTRL0 |= (1 << 0); //enable interrupt
  MRT_CTRL0 &= ~(0x03 << 1); //repeat interrupt mode
}
//////////////////////////////////////////////////////////////////////////

void usart0SendArr(uint8_t *data, uint16_t len) {
  disableRxReady();
  while(len--)
    usart0SendSync(*(data++));
  enableRxReady();
}
//////////////////////////////////////////////////////////////////////////
