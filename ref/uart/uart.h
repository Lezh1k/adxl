#ifndef UART_H
#define UART_H


class ArmUart;
class Uart
{
public:
    static Uart *u0();
    void putch(char sym);
    int getch();   //WAIT!
    bool kbhit();
    char getKey();  //return 0 if no char;

    void write(const char *buffer, unsigned int size);
    void puts(const char *str);
    void sendBreak();

    void printf(const char *format,...);
    void printfn(const char *format,...);

    void hexByte(unsigned char b);
    void hexUint(unsigned int v);

    int readyCount()const;

    int rxFreeSymvols()const;

private:
    Uart() = delete;
    Uart(int num);
    ArmUart *m_armUart;

    char m_uartRxBuf[1024*2];
    char m_uartTxBuf[128];
};

extern Uart *u;


#endif // UART_H
