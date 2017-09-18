#include "modbussimple.h"
#include "uart.h"
#include <string.h>

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



ModBusSimple::ModBusSimple(int mySerial):
    m_serial(mySerial),
    m_pos(0)
{

}

const char *ModBusSimple::extractPack()
{
    m_pos = 0;
    return m_pack;
}

void ModBusSimple::sendPack(unsigned char funcNum, unsigned int address, unsigned int value)
{
    char buf[10];

    buf[0] = m_serial;
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

    buf[0] = m_serial;
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


char ModBusSimple::getSerial() const
{
    return m_serial;
}

void ModBusSimple::setSerial(char serial)
{
    m_serial = serial;
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

bool ModBusSimple::testPack()
{
    if(m_pos < 4 || m_pos>= sizeof(m_pack))
        return false;
    if(m_pack[0] != m_serial && m_pack[0] != 0)
        return false;
    unsigned int crc = crc16(m_pack,m_pos-2);
    unsigned int p = static_cast<unsigned char>(m_pack[m_pos-2]);
    p |= (static_cast<unsigned char>(m_pack[m_pos-1])<<8);
    if(p!=crc)
        return false;
    return true;
}



int ModBusSimple::getPackLen()
{
    while(u->kbhit()){
        int c = u->getch();
        if(c == -2){
            if(m_pos != sizeof(m_pack)){
                if(testPack()){
                    return m_pos;
                }
            }
            m_pos = 0;
        }
        if(m_pos < sizeof(m_pack)){
            m_pack[m_pos++] = c;
        }
    }
    return 0;
}
