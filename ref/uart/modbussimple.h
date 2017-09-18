#ifndef MODBUSSIMPLE_H
#define MODBUSSIMPLE_H


class ModBusSimple
{
public:
    ModBusSimple(int mySerial);

    int getPackLen();//принимаем, проверяем, если нет - то 0
    const char *extractPack();//зануляем буфер
    void sendPack(unsigned char funcNum, unsigned int address, unsigned int value);
    void sendError(unsigned char funcNum, unsigned int errCode);

    void test();

    char getSerial() const;

    void setSerial(char serial);

private:
    char m_serial;
    char m_pack[256];
    unsigned int m_pos;

    bool testPack();

    void sendDataWithCrc(char *data, int len);
};

#endif // MODBUSSIMPLE_H
