#ifndef ARMUART_H
#define ARMUART_H

#include <stdint.h>


class ArmUart
{
public:
    ArmUart(int num, int baud, char *rxBuffer, int rxSize, char *txBuffer, int txSize);
    ~ArmUart();

    void putch(char sym);
    void sendBreak();
    void uartInterrupt0();//rxReady, txready, txidle
    void tick();//tickrx, ticktx
    int  getch();   //-2 = break
    bool kbhit();
    int  readyCount();//не учитывает размер бряков и т.п. то есть покажет больше чем есть на самом деле

    int rxFreeTicks()const;
private:
    //ArmUart() = delete;
    ArmUart(const ArmUart &) = delete;
    ArmUart &operator=(const ArmUart &) = delete;
    inline void setPin485(bool isTransmit);
    void realSend(char c);
    void checkTxAction();

    void putToTx(char c);
    void putToRx(char c);
    void putSpecToRx(char c);
    int  getFromTx();//sym, -1 = end, -2 = stop


    volatile unsigned int rx_insert;
    volatile unsigned int rx_extract;
    volatile unsigned int tx_insert;
    volatile unsigned int tx_extract;
    volatile bool m_isTx;
    volatile bool m_isWait;
    volatile bool m_isNeedRxCounterClear;

    int m_counterRx;
    int m_counterTx;

    unsigned char *rx_buffer;
    unsigned char *tx_buffer;

    unsigned int rx_size;
    unsigned int tx_size;
};

#endif // ARMUART_H
