#include "modbussimple.h"
#include "uart.h"
#include <string.h>

#define DEBUG 0
#if DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif


unsigned int crc16(const char *buf, int len)
{

    unsigned int crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (unsigned int)(unsigned char)(*buf++);          // XOR byte into least sig. byte of crc
        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }else{                            // Else LSB is not set
                crc >>= 1;                    // Just shift right
            }

        }
    }

    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc; //(crc>>8) | ((crc&0xff)<<8);
}


ModBusSimple::ModBusSimple(int mySerial, int weakBroadcast):
    m_serial(mySerial),
    m_weakBroadcast(weakBroadcast),
    m_answerId(m_serial),
    m_pos(0)
{

}


const char *ModBusSimple::extractPack()
{
    m_pos = 0;
    return m_pack;
}

void ModBusSimple::sendAknPack(unsigned char funcNum, unsigned int address, unsigned int value)
{
    if(m_answerId == 0 || m_answerId == m_weakBroadcast)
        return;//на бродкасты не отвечаем
    char buf[10];

    buf[0] = m_answerId;
    buf[1] = funcNum;
    buf[2] = address>>8;
    buf[3] = address&0xff;
    buf[4] = value>>8;
    buf[5] = value&0xff;
    sendDataWithCrc(buf,6);
}

void ModBusSimple::sendError(unsigned char funcNum, unsigned int errCode)
{
    char buf[10];

    buf[0] = m_answerId;
    buf[1] = funcNum | 0x80;
    buf[2] = errCode;
    sendDataWithCrc(buf,3);
}

void ModBusSimple::sendDataWithCrc(char *data, int len)
{
    unsigned int crc = crc16(data,len);
    u->sendBreak();
    u->write(data,len);
    u->putch(crc & 0xff);
    u->putch(crc >> 8);
    u->sendBreak();
}


char ModBusSimple::getId() const
{
    return m_serial;
}

void ModBusSimple::setId(char id)
{
    m_serial = id;
}

void ModBusSimple::partStart(unsigned char funcNum)
{
    m_pack[0] = m_answerId;
    m_pack[1] = funcNum;
    m_sendRegCount = 0;
}

void ModBusSimple::partNextReg(unsigned int value)
{
    unsigned int p = (m_sendRegCount++)*2 + 3;
    if(p>sizeof(m_pack)-3)
        return;
    m_pack[p] = value>>8;
    m_pack[p+1] = value&0xff;
}

void ModBusSimple::partSend()
{
    int len = m_sendRegCount*2;
    m_pack[2] = len;
    len += 3;
    sendDataWithCrc(m_pack,len);
}

char ModBusSimple::getWeakBroadcast() const
{
    return m_weakBroadcast;
}

void ModBusSimple::setWeakBroadcast(char weakBroadcast)
{
    m_weakBroadcast = weakBroadcast;
}

char ModBusSimple::getAnswerId() const
{
    return m_answerId;
}

void ModBusSimple::setAnswerId(char answerId)
{
    m_answerId = answerId;
}

//void ModBusSimple::test()
//{
//    char b1[] = {0x01,0x0F,0x00,0x13,0x00,0x0A,0x02,0xCD,0x01,0x72,0xCB};
//    char b2[] = {0x01,0x0F,0x00,0x13,0x00,0x0A,0x24,0x09};
//    char b3[] = {0x01,0x8F,0x02,0xC5,0xF1};

//    m_pos = sizeof(b1);memcpy(m_pack,b1,m_pos);
//    u->printfn("Test=%b, crc=%X",testPack(),crc16(m_pack,m_pos-2));
//    m_pos = sizeof(b2);memcpy(m_pack,b2,m_pos);
//    u->printfn("Test=%b, crc=%X",testPack(),crc16(m_pack,m_pos-2));
//    m_pos = sizeof(b3);memcpy(m_pack,b3,m_pos);
//    u->printfn("Test=%b, crc=%X",testPack(),crc16(m_pack,m_pos-2));

//    //sendPack(0x0f,0x13,0x0a);
//}

#if DEBUG
bool badbad(const char *reason){u->printf("[-%s-]",reason);return false;}
#else
#define badbad(a) false
#endif

bool ModBusSimple::testPack()
{
    if(m_pos < 4 || m_pos>= sizeof(m_pack))
        return badbad("size");

    DBG(u->puts("[PACK=");
    for(unsigned int i=0;i<m_pos;i++){
        u->hexByte(m_pack[i]);
        u->puts(" ");
    }
    u->puts("] "););

    if(m_pack[0] != m_serial && m_pack[0] != 0 && (m_weakBroadcast && m_pack[0] != m_weakBroadcast)){
        DBG(u->printfn("[adr=%d,s=%d,w=%d]",m_pack[0],m_serial,m_weakBroadcast));
        return badbad("address");
    }
    unsigned int crc = crc16(m_pack,m_pos-2);
    unsigned int p = static_cast<unsigned char>(m_pack[m_pos-2]);
    p |= (static_cast<unsigned char>(m_pack[m_pos-1])<<8);
    if(p!=crc)
        return badbad("crc");

    unsigned char func = m_pack[1];
    if(func & 0x80)
        return badbad("errfunc");       // это ошибка другого устройства
    if(func == 3 || func == 4){
        if(m_pos & 1)
            return badbad("other34");;   //это ответ других
    }
    if(func == 0x0F || func == 0x10){
        if(!(m_pos & 1))
            return badbad("other1516");;   //это ответ других
    }


    return true;
}



int ModBusSimple::getPackLen()
{
    if(m_isAlreadyPack){
        m_isAlreadyPack = false;
        if(testPack())
            return m_pos;
        return 0;
    }
    while(u->kbhit()){
        int c = u->getch();
        if(c == -2){
            DBG(u->puts("<##>"));
            if(m_pos != sizeof(m_pack)){
                if(testPack()){
                    DBG(u->printf("{%d}",m_pos));
                    return m_pos;
                }else{
                    DBG(u->puts("{--}"));
                }
            }
            m_pos = 0;
        }else{
            DBG(u->putch('<'));
            DBG(u->hexByte(c));
            DBG(u->putch('>'));
            if(m_pos < sizeof(m_pack)){
                m_pack[m_pos++] = c;
            }
        }
    }
    return 0;
}

int ModBusSimple::insertNewPack(const unsigned char *buf, int len)
{
    m_isAlreadyPack = true;
    while(len--){
        int c = *buf++;
        if(m_pos < sizeof(m_pack)){
            m_pack[m_pos++] = c;
        }
    }

    if(m_pos != sizeof(m_pack)){
        if(testPack()){
            return m_pos;
        }else{
            u->puts("!test:");
            for(unsigned int i=0;i<m_pos;i++){
                u->hexByte(m_pack[i]);
                u->putch(' ');
            }
            u->puts("\r\n");
        }
    }
    m_pos = 0;

    return 0;
}
