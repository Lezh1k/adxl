#include "baserssensor.h"
#include "basersprocess.h"
#include "KobusTypes.h"

BaseRsSensor::BaseRsSensor():
    m_parent(0),
    m_fullContext(-2)
{

}

BaseRsSensor::~BaseRsSensor()
{
    if(m_parent)
        m_parent->removeSensor(this);
}

void BaseRsSensor::setParent(BaseRsProcess *parent)
{
    m_parent = parent;
}

void BaseRsSensor::setFullContext(int context_)
{
    m_fullContext = context_;
    setSensorContext(m_fullContext / 256);
}

int BaseRsSensor::fullContext() const
{
    return m_fullContext;
}

void BaseRsSensor::sendByMeRaw(const unsigned char *buf, int len)const
{
    rsSend(buf,len,true);
}


void BaseRsSensor::setSensorContext(int context)
{
    Q_UNUSED(context);
}

void BaseRsSensor::rsSend(const unsigned char *buf, int len, bool isRaw)const
{
    if(m_parent)
        m_parent->send(buf,len,isRaw);
}
