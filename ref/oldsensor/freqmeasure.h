#ifndef FREQMEASURE_H
#define FREQMEASURE_H


class FreqMeasure
{
public:
    FreqMeasure();
    int receiveFrequence();
    int countReceivesPerSec() const;
    void setCountReceives(int countReceives_);
    unsigned int countReceivesTotal() const;

protected:
    void receiveNext();
private:
    int m_countReceivesPerSec;
    unsigned long m_prevTick;
    int m_prevFreq;
    unsigned int m_countReceivesTotal;
};

#endif // FREQMEASURE_H
