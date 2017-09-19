#ifndef BASERSPROCESS_H
#define BASERSPROCESS_H


class BaseRsSensor;
class DigChannel;
class BaseRsProcess{
public:
    BaseRsProcess();
    virtual ~BaseRsProcess();

    void addSensor(BaseRsSensor *sensor);
    void removeSensor(BaseRsSensor *sensor);
    void send(const unsigned char *buf, int len, bool isRaw) const;
    void setParent(DigChannel *parent);
    virtual void receiveChar(unsigned char ch, bool is485) = 0;
protected:
    void sendRaw(const unsigned char *buf, int len) const;
    void receiveToAll(const unsigned char *buf, int len, bool is485);
    virtual void sendPrepared(const unsigned char *buf, int len)const = 0;
private:

    BaseRsSensor *m_list[5];
    DigChannel *m_parent;
};

class RsProcessDirect :
        public BaseRsProcess
{
public:
    RsProcessDirect();
    void receiveChar(unsigned char ch, bool is485);
private:
    void sendPrepared(const unsigned char *buf, int len)const;
};



#endif // BASERSPROCESS_H
