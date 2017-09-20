#ifndef MODBUSSIMPLE_H
#define MODBUSSIMPLE_H


class ModBusSimple
{
public:
    enum Errors{
        ErrNoError = 0,
        ErrUnknownFuncCode,
        ErrAddressUnavailable,
        ErrBadValue,
        ErrFatalAction,
        ErrInProcess,
        ErrBusy,
        ErrCantFunction,
        ErrHardFault,
    };

    ModBusSimple(int mySerial, int weakBroadcast);

    int insertNewPack(const unsigned char *buf, int len);

    int getPackLen();//принимаем, проверяем, если нет - то 0
    const char *extractPack();//зануляем буфер
    void sendAknPack(unsigned char funcNum, unsigned int address, unsigned int value);
    void sendError(unsigned char funcNum, unsigned int errCode);

    void test();

    char getId() const;

    void setId(char id);


    void partStart(unsigned char funcNum);
    void partNextReg(unsigned int value);
    void partSend();

    char getWeakBroadcast() const;
    void setWeakBroadcast(char weakBroadcast);

    char getAnswerId() const;
    void setAnswerId(char answerId);

private:
    int m_sendRegCount;
    char m_serial;
    char m_weakBroadcast;
    char m_pack[256];
    char m_answerId;
    unsigned int m_pos;
    bool m_isAlreadyPack = false;

    bool testPack();

    void sendDataWithCrc(char *data, int len);
};


//usage ChangeId(bus,32)->sendPack(1,2,3);
class ChangeId{
public:
    ChangeId(ModBusSimple *bus, int serial):m_bus(bus),m_wasId(bus->getId()){bus->setId(serial);}
    ~ChangeId(){m_bus->setId(m_wasId);}
    inline ModBusSimple *operator->(){return m_bus;}
private:
    ModBusSimple *m_bus;
    int m_wasId;
};




#endif // MODBUSSIMPLE_H
