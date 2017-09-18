#include "armuart.h"
#include "chip.h"

// DLE-DLE = DLE
// DLE-STP = сделать дыру / была дыра
#define UART_BR_DLE 0x72
#define UART_BR_STP 0x01

#define UART_BR_TIMEWAIT 6
#define UART_BR_TIMEWAITRX 4

#define ENABLETX0    LPC_USART0->INTENSET = UART_INTEN_TXRDY
#define DISABLETX0   LPC_USART0->INTENCLR = UART_INTEN_TXRDY
#define ENABLEIDLE0  LPC_USART0->INTENSET = UART_INTEN_TXIDLE
#define DISABLEIDLE0 LPC_USART0->INTENCLR = UART_INTEN_TXIDLE
#define RAMFUNC __attribute__ ((long_call, section (".ramfunctions")))

static ArmUart *armUart0 = 0;

extern "C" RAMFUNC void UART0_IRQHandler(void)
{
    if(armUart0)
        armUart0->uartInterrupt0();
}

RAMFUNC void uart0doEvery50ms(void)
{
    if(armUart0)
        armUart0->tick();
}

ArmUart::ArmUart(int num, int baud, char *rxBuffer, int rxSize, char *txBuffer, int txSize)
{
    rx_insert = rx_extract = tx_insert = tx_extract = 0;

    rx_size = rxSize;

    tx_size = txSize;

    rx_buffer = reinterpret_cast<unsigned char*>(rxBuffer);
    tx_buffer = reinterpret_cast<unsigned char*>(txBuffer);

    m_isTx = false;
    m_isWait = false;
    m_counterRx = 0;
    m_counterTx = 0;
    m_isNeedRxCounterClear = false;


    if(num!=0)
        return;



    // Setup UART
    Chip_UART_Init(LPC_USART0);
    Chip_UART_ConfigData(LPC_USART0, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_Clock_SetUSARTNBaseClockRate((baud * 16), true);
    Chip_UART_SetBaud(LPC_USART0, baud);
    Chip_UART_Enable(LPC_USART0);
    Chip_UART_TXEnable(LPC_USART0);

    LPC_USART0->INTENCLR = 0xFD6D;

    // Enable receive data and line status interrupt
    Chip_UART_IntEnable(LPC_USART0, UART_INTEN_RXRDY);
    DISABLETX0;
    DISABLEIDLE0;


    // initialize the interrupt vector
    NVIC_SetPriority(UART0_IRQn, 25);//preemption 25 --- LOWEST PRIORITY INT
    NVIC_EnableIRQ(UART0_IRQn);

    armUart0 = this;

}

ArmUart::~ArmUart()
{
    armUart0 = 0;
}

void ArmUart::putch(char sym)
{
    if(!m_isTx){
        m_isTx = true;
        if(!m_isWait){
            realSend(sym);
            return;
        }
    }
    if(sym == UART_BR_DLE)
        putToTx(sym);
    putToTx(sym);
}

void ArmUart::sendBreak()
{
    putToTx(UART_BR_DLE);
    putToTx(UART_BR_STP);
}

RAMFUNC void ArmUart::uartInterrupt0()
{
    unsigned int iid = LPC_USART0->INTSTAT;
    if(iid & UART_STAT_RXRDY){
        putToRx(LPC_USART0->RXDATA & 0xFF);
        m_isNeedRxCounterClear = true;
    }
    if(iid & UART_STAT_TXRDY){
       checkTxAction();
    }
    if(iid & UART_STAT_TXIDLE){
        DISABLEIDLE0;
        setPin485(false);
    }
}

RAMFUNC void ArmUart::tick()
{
    int cntrx = m_counterRx;
    if(m_isNeedRxCounterClear){
        cntrx = 0;
        m_isNeedRxCounterClear = false;
    }else{
        cntrx++;
    }

    if(cntrx==UART_BR_TIMEWAITRX){
        putSpecToRx(UART_BR_STP);
    }else{
        cntrx &= 0x0FFFFFFF;//to not overflow
    }
    m_counterRx = cntrx;



    if(m_isWait){
        if(++m_counterTx > UART_BR_TIMEWAIT){
            if(m_counterRx > UART_BR_TIMEWAIT){//wait for free bus
                m_isWait = false;
                if(m_isTx){
                    checkTxAction();
                }
            }
        }
    }else{
        m_counterTx = 0;
    }

}

int ArmUart::getch()
{
    while(rx_insert == rx_extract){};

    unsigned int tmp = rx_extract;

    if(++tmp >= rx_size)
        tmp = 0;
    int ch = rx_buffer[tmp];

    if(ch == UART_BR_DLE){
        while(tmp == rx_extract){};
        if(++tmp >= rx_size)
            tmp = 0;
        ch = rx_buffer[tmp];
        if(ch==UART_BR_STP)
            ch = -2;
    }
    rx_extract = tmp;
    return ch;
}

bool ArmUart::kbhit()
{
    return rx_insert != rx_extract;
}

int ArmUart::readyCount()
{
    int space;

    unsigned int tmp = rx_insert;

    if ((space = (tmp - rx_extract)) < 0)
        space += rx_size;

    return space;
}

int ArmUart::rxFreeTicks() const
{
    return m_counterRx;
}

RAMFUNC inline void ArmUart::setPin485(bool isTransmit)
{
    (void)isTransmit;
}

RAMFUNC void ArmUart::realSend(char c)
{
    setPin485(true);
    LPC_USART0->TXDATA = (unsigned int) c;
    ENABLETX0;
}

RAMFUNC void ArmUart::checkTxAction()
{
    int ch = getFromTx();
    if(ch>=0){
        realSend(ch);
    }else if(ch==-1){
        m_isTx = false;
        DISABLETX0;
        ENABLEIDLE0;
    }else if(ch==-2){
        m_isWait = true;
        DISABLETX0;
        ENABLEIDLE0;
    }
}

void ArmUart::putToTx(char c)
{
    for(;;){
        unsigned int tmp = tx_insert + 1;
        if(tmp >= tx_size)
            tmp = 0;
        if(tmp == tx_extract)
            continue;
        tx_buffer[tmp] = c;
        tx_insert = tmp;
        break;
    }
}

RAMFUNC void ArmUart::putSpecToRx(char c)
{
    unsigned int tmp0 = rx_insert + 1;
    if(tmp0 >= rx_size)
        tmp0 = 0;
    if(tmp0 == rx_extract)
        return;
    unsigned int tmp1 = tmp0+1;
    if(tmp1 >= rx_size)
        tmp1 = 0;
    if(tmp1 == rx_extract)
        return;
    rx_buffer[tmp0] = UART_BR_DLE;
    rx_buffer[tmp1] = c;
    rx_insert = tmp1;
}

RAMFUNC int ArmUart::getFromTx()
{
    unsigned int tmp = tx_extract;
    if(tx_insert != tmp){
        if(++tmp >= tx_size)
            tmp = 0;
        int ch = tx_buffer[tmp];
        if(ch == UART_BR_DLE){
            if(tx_insert != tmp){
                if(++tmp >= tx_size)
                    tmp = 0;
                ch = tx_buffer[tmp];
                if(ch == UART_BR_STP)
                    ch = -2;
            }else{
                ch = -1;
            }
        }
        tx_extract = tmp;
        return ch;
    }
    return -1;
}

RAMFUNC void ArmUart::putToRx(char c)
{
    if(c == UART_BR_DLE){
        putSpecToRx(c);
    }else{
        unsigned int tmp = rx_insert + 1;
        if(tmp >= rx_size)
            tmp = 0;
        if(tmp == rx_extract)
            return;
        rx_buffer[tmp] = c;
        rx_insert = tmp;
    }
}


