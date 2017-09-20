#include "armuart.h"
#include "lpc824.h"
#include "commons.h"

// DLE-DLE = DLE
// DLE-STP = сделать дыру / была дыра
#define UART_BR_DLE 0x72
#define UART_BR_STP 0x01

#define UART_BR_TIMEWAIT 6
#define UART_BR_TIMEWAITRX 4

static inline void enableRxReady() { USART0_INTENSET |= (1 << 0); }
static inline void enableTxReady() { USART0_INTENSET |= (1 << 2); }
static inline void enableTxIdle()  { USART0_INTENSET |= (1 << 3); }

static inline void disableRxReady() { USART0_INTENCLR |= (1 << 0); }
static inline void disableTxReady() { USART0_INTENCLR |= (1 << 2); }
static inline void disableTxIdle()  { USART0_INTENCLR |= (1 << 3); }

#define ENABLETX0    (USART0_INTENSET |= (1 << 2))
#define DISABLETX0   (USART0_INTENCLR |= (1 << 2))
#define ENABLEIDLE0  (USART0_INTENSET |= (1 << 3))
#define DISABLEIDLE0 (USART0_INTENCLR |= (1 << 3))
#define RAMFUNC __attribute__ ((long_call, section (".ramfunctions")))

#define FALSE 0
#define TRUE 1

typedef uint8_t bool; //change
static inline void setPin485(bool isTransmit);
static void realSend(char c);
static void checkTxAction();

static void putToTx(char c);
static void putToRx(char c);
static void putSpecToRx(char c);
static int32_t getFromTx();//sym, -1 = end, -2 = stop

static volatile uint32_t rx_insert;
static volatile uint32_t rx_extract;
static volatile uint32_t tx_insert;
static volatile uint32_t tx_extract;

static volatile bool m_isTx;
static volatile bool m_isWait;
static volatile bool m_isNeedRxCounterClear;

static int32_t m_counterRx;
static int32_t m_counterTx;

static uint8_t *rx_buffer;
static uint8_t *tx_buffer;
static uint32_t rx_size;
static uint32_t tx_size;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

RAMFUNC void USART0_IRQHandler(void) {
  armUartInterrupt0();
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void uart0doEvery50ms(void) {
  armUartTick();
}
//////////////////////////////////////////////////////////////////////////

#define USART0_BAUD_RATE 19200
void armUart(char *rxBuffer,
             int32_t rxSize,
             char *txBuffer,
             int32_t txSize) {

  rx_insert = rx_extract = tx_insert = tx_extract = 0;
  rx_size = rxSize;
  tx_size = txSize;

  rx_buffer = (unsigned char*)rxBuffer;
  tx_buffer = (unsigned char*)txBuffer;

  m_isTx = FALSE;
  m_isWait = FALSE;
  m_counterRx = 0;
  m_counterTx = 0;
  m_isNeedRxCounterClear = FALSE;

  SWM_PINASSIGN0 = (0x0e << 0)  | //U0_TXD -> PIO0_14
                   (0x0b << 8)  | //U0_RXD -> PIO0_11
                   (0x0f << 16) | //U0_RTS -> PIO0_15 . RTS should be used as DE.
                   (0xff << 24);  //disable U0_DTS

  SYSCON_PRESETCTRL &= ~(1 << 3); //reset USART0
  SYSCON_SYSAHBCLKCTRL |= (1 << 14); //enable clock for USART0
  SYSCON_PRESETCTRL |= (1 << 3); //take USART0 out of reset

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

  // Setup UART
//  Chip_UART_TXEnable(LPC_USART0);

  USART0_INTENCLR = 0xFD6D; //todo what's this? 1111 1101 0110 1101
  enableRxReady();
  DISABLETX0;
  DISABLEIDLE0;

  NVIC_ISER0 |= (1 << 3); //enable USART0 IRQ
  NVIC_IPR0 |= (0x03 << 6); //USART0 lowest priority int
//  armUart0 = this;
}
//////////////////////////////////////////////////////////////////////////

void armUartDelete() {
  //armUart0 = 0;
}
//////////////////////////////////////////////////////////////////////////

void armUartPutch(char sym) {
  if(!m_isTx){
    m_isTx = TRUE;
    if(!m_isWait){
      realSend(sym);
      return;
    }
  }
  if(sym == UART_BR_DLE)
    putToTx(sym);
  putToTx(sym);
}
//////////////////////////////////////////////////////////////////////////

void armUartSendBreak() {
  putToTx(UART_BR_DLE);
  putToTx(UART_BR_STP);
}
//////////////////////////////////////////////////////////////////////////

void armUartInterrupt0() {
  uint32_t iid = USART0_INTSTAT;
  if(iid & (1 << 0)) {
    putToRx(USART0_RXDAT & 0xFF);
    m_isNeedRxCounterClear = TRUE;
  }

  if(iid & (1 << 2)) {
    checkTxAction();
  }

  if(iid & (1 << 3)){
    DISABLEIDLE0;
    setPin485(FALSE);
  }
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void armUartTick() {
  int cntrx = m_counterRx;
  if(m_isNeedRxCounterClear) {
    cntrx = 0;
    m_isNeedRxCounterClear = FALSE;
  } else {
    cntrx++;
  }

  if(cntrx==UART_BR_TIMEWAITRX) {
    putSpecToRx(UART_BR_STP);
  } else {
    cntrx &= 0x0FFFFFFF;//to not overflow
  }
  m_counterRx = cntrx;

  if(m_isWait) {
    if(++m_counterTx > UART_BR_TIMEWAIT){
      if(m_counterRx > UART_BR_TIMEWAIT){//wait for free bus
        m_isWait = FALSE;
        if(m_isTx){
          checkTxAction();
        }
      }
    }
  } else {
    m_counterTx = 0;
  }
}
//////////////////////////////////////////////////////////////////////////

int32_t armUartGetch() {
  uint32_t tmp;
  while(rx_insert == rx_extract) ;
  tmp = rx_extract;

  if(++tmp >= rx_size)
    tmp = 0;
  int ch = rx_buffer[tmp];

  if(ch == UART_BR_DLE) {
    while(tmp == rx_extract) ;
    if(++tmp >= rx_size)
      tmp = 0;
    ch = rx_buffer[tmp];
    if(ch==UART_BR_STP)
      ch = -2;
  }
  rx_extract = tmp;
  return ch;
}
//////////////////////////////////////////////////////////////////////////

uint8_t armUartKbhit() {
  return rx_insert != rx_extract;
}
//////////////////////////////////////////////////////////////////////////

int32_t armUartReadyCount() {
  int32_t space;
  uint32_t tmp = rx_insert;

  if ((space = (tmp - rx_extract)) < 0)
    space += rx_size;

  return space;
}
//////////////////////////////////////////////////////////////////////////

int32_t armUartRxFreeTicks() {
  return m_counterRx;
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC inline void setPin485(bool isTransmit) {
  UNUSED_ARG(isTransmit);
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void realSend(char c) {
  setPin485(TRUE);
  USART0_TXDAT = (uint32_t) c;
  ENABLETX0;
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void checkTxAction() {
  int32_t ch = getFromTx();
  if( ch >= 0) {
    realSend(ch);
  } else if (ch == -1) {
    m_isTx = FALSE;
    DISABLETX0;
    ENABLEIDLE0;
  } else if (ch == -2) {
    m_isWait = TRUE;
    DISABLETX0;
    ENABLEIDLE0;
  }
}
//////////////////////////////////////////////////////////////////////////

void putToTx(char c) {
  for(;;) {
    uint32_t tmp = tx_insert + 1;
    if (tmp >= tx_size)
      tmp = 0;
    if (tmp == tx_extract)
      continue;
    tx_buffer[tmp] = c;
    tx_insert = tmp;
    break;
  }
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void putToRx(char c) {
  if(c == UART_BR_DLE) {
    putSpecToRx(c);
  } else {
    unsigned int tmp = rx_insert + 1;
    if(tmp >= rx_size)
      tmp = 0;
    if(tmp == rx_extract)
      return;
    rx_buffer[tmp] = c;
    rx_insert = tmp;
  }
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC void putSpecToRx(char c) {
  uint32_t tmp0, tmp1;
  tmp0 = rx_insert + 1;
  if (tmp0 >= rx_size)
    tmp0 = 0;
  if (tmp0 == rx_extract)
    return;
  tmp1 = tmp0+1;
  if (tmp1 >= rx_size)
    tmp1 = 0;
  if (tmp1 == rx_extract)
    return;
  rx_buffer[tmp0] = UART_BR_DLE;
  rx_buffer[tmp1] = c;
  rx_insert = tmp1;
}
//////////////////////////////////////////////////////////////////////////

RAMFUNC int32_t getFromTx() {//sym, -1 = end, -2 = stop
  uint32_t tmp = tx_extract;
  int32_t ch;

  if (tx_insert != tmp) {
    if(++tmp >= tx_size)
      tmp = 0;
    ch = tx_buffer[tmp];
    if (ch == UART_BR_DLE) {
      if (tx_insert != tmp){
        if (++tmp >= tx_size)
          tmp = 0;
        ch = tx_buffer[tmp];
        if (ch == UART_BR_STP)
          ch = -2;
      } else {
        ch = -1;
      }
    } //ch == UART_BR_DLE
    tx_extract = tmp;
    return ch;
  }
  return -1;
}
//////////////////////////////////////////////////////////////////////////
