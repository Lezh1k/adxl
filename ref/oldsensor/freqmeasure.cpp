#include "freqmeasure.h"
#include "interrupts/timer_independent.h"

FreqMeasure::FreqMeasure():
    m_countReceivesPerSec(0),
    m_prevTick(TickVar),
    m_prevFreq(0),
    m_countReceivesTotal(0)
{

}

int FreqMeasure::receiveFrequence()
{
    unsigned long t = TickVar - m_prevTick;
    if(t > 1000){
        m_prevTick = TickVar;
        m_prevFreq = m_countReceivesPerSec * 1000 / t;
        m_countReceivesPerSec = 0;
    }
    return m_prevFreq;
}

int FreqMeasure::countReceivesPerSec() const
{
    return m_countReceivesPerSec;
}

void FreqMeasure::setCountReceives(int countReceives_)
{
    m_countReceivesPerSec = countReceives_;
}

void FreqMeasure::receiveNext()
{
    m_countReceivesPerSec++;
    m_countReceivesTotal++;
}
unsigned int FreqMeasure::countReceivesTotal() const
{
    return m_countReceivesTotal;
}



