#ifndef BASERSSENSOR_H
#define BASERSSENSOR_H


class BaseRsProcess;
class BaseRsSensor{
public:
    BaseRsSensor();
    virtual ~BaseRsSensor();

    void setParent(BaseRsProcess *parent);
    void setFullContext(int context_);
    int fullContext()const;
    virtual void rsReceive(const unsigned char *buf, int len, bool is485) = 0;
    void sendByMeRaw(const unsigned char *buf, int len) const;
protected:
    virtual void setSensorContext(int context);
    void rsSend(const unsigned char *buf, int len, bool isRaw) const;
private:
    BaseRsProcess *m_parent;
    int m_fullContext;
};

#endif // BASERSSENSOR_H
