#include "uart.h"
#include "uart/armuart.h"
#include <stdint.h>
#include <string.h>
#include "../../../kobus7/shared/StrIntFloat.h"

#ifndef REDUCE_UART_PRINTF
#include <stdarg.h>
#endif

#define RAMFUNC __attribute__ ((long_call, section (".ramfunctions")))


Uart::Uart(int num):
    m_armUart(0)
{
    if(num==0){
        static ArmUart au(0,115200,m_uartRxBuf, sizeof(m_uartRxBuf), m_uartTxBuf, sizeof(m_uartTxBuf));
        m_armUart = &au;
        u = this;
    }
}


Uart *Uart::u0()
{
    static Uart me(0);
    return &me;
}


void Uart::putch(char sym)
{
    m_armUart->putch(sym);
}

int Uart::getch()
{
    return m_armUart->getch();
}

bool Uart::kbhit()
{
    return m_armUart->kbhit();
}

char Uart::getKey()
{
    while(kbhit()){
        int c = getch();
        if(c >= 0)
            return c;
    }
    return 0;
}

void Uart::write(const char *buffer, unsigned int size)
{
    for(;size>0;size--){
        putch(*buffer++);
    }
}

void Uart::puts(const char *str)
{
    for(;str && *str;)
        putch(*str++);
}

void Uart::sendBreak()
{
    m_armUart->sendBreak();
}


#ifndef va_arg
#define va_arg(a,z) (z)(a)
#endif


#ifndef REDUCE_UART_PRINTF
void Uart::printf(const char *format,...)
{
    if(!format || !*format)
        return;
    va_list ap;
    va_start(ap, format);
    for(;*format;format++){
        if(*format!='%'){
            putch(*format);
        }else{
            switch (*++format) {
            case 0:
                return;
            case 's':{
                const char *s = va_arg(ap, char *);
                if(s)
                    puts(s);
                else
                    puts("(null)");
                break;
            }
            case 'd':{
                int d = va_arg(ap, int);
                char buf[18];
                puts(m_intToString(buf,d));
                break;
            }
            case 'c': {
                putch((char) va_arg(ap, int));
                break;
            }
            case 'x':{
                hexByte((char) va_arg(ap, int));
                break;
            }
            case 'X':{
                hexUint(va_arg(ap, int));
                break;
            }
            case '%': {
                putch('%');
                break;
            }
            case 'b':{
                puts(va_arg(ap, int) ? "true" : "false");
                break;
            }
            default:
                puts("<WRONG%:");
                putch(*format);
                puts(">");
            }


        }
    }

    va_end(ap);

}




void Uart::printfn(const char *format,...)
{
    if(!format || !*format){
        puts("\r\n");
        return;
    }
    va_list ap;
    va_start(ap, format);
    for(;*format;format++){
        if(*format!='%'){
            putch(*format);
        }else{
            switch (*++format) {
            case 0:
                return;
            case 's':{
                const char *s = va_arg(ap, char *);
                if(s)
                    puts(s);
                else
                    puts("(null)");
                break;
            }
            case 'd':{
                int d = va_arg(ap, int);
                char buf[18];
                puts(m_intToString(buf,d));
                break;
            }
            case 'c': {
                putch((char) va_arg(ap, int));
                break;
            }
            case 'x':{
                hexByte((char) va_arg(ap, int));
                break;
            }
            case 'X':{
                hexUint(va_arg(ap, int));
                break;
            }
            case '%': {
                putch('%');
                break;
            }
            case 'b':{
                puts(va_arg(ap, int) ? "true" : "false");
                break;
            }
            default:
                puts("<WRONG%:");
                putch(*format);
                puts(">");
            }


        }
    }

    va_end(ap);

    puts("\r\n");
}
#else
void Uart::printf(const char *format,...){puts(format);}
void Uart::printfn(const char *format,...){puts(format);puts("\r\n");}

#endif

void Uart::hexByte(unsigned char b)
{
    char buf[3];
    buf[2] = 0;
    puts(m_btoh(buf,b));
}

void Uart::hexUint(unsigned int v)
{
    char buf[9];
    m_btoh(buf + 0, (v>>24)&0xFF);
    m_btoh(buf + 2, (v>>16)&0xFF);
    m_btoh(buf + 4, (v>>8 )&0xFF);
    m_btoh(buf + 6, (v    )&0xFF);
    buf[8] = 0;
    puts(buf);
}

int Uart::readyCount() const
{
    return m_armUart->readyCount();
}

int Uart::rxFreeSymvols() const
{
    return m_armUart->rxFreeTicks()>>2;
}



Uart *u = 0;
