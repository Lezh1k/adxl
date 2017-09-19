#ifndef SENSORDIGITALANGLE_H
#define SENSORDIGITALANGLE_H

#ifndef DEBUG_TEST_TOOL
#include "KobusTypes.h"
#endif
#include "baserssensor.h"
#include "freqmeasure.h"


#define angleGroupTower         "AnglT"
#define angleGroupPlatform      "AnglP"

#define angleVarOffsetLong      "offsL"
#define angleVarOffsetCross     "offsC"
#define angleVarCorrKLong       "kL"
#define angleVarCorrKCross      "kC"

class SensorDigitalAngle :
        public FreqMeasure,
        public BaseRsSensor
{
public:
    static SensorDigitalAngle*instance();
    SensorDigitalAngle(const char *groupVar_);
    float crossAngle()const;
    float longAngle()const;


    bool isAnglePresent() const;


    enum Cmds {
        RequestAngle,
        TurnOffAutoSend,
        TurnOnAutoSend5Hz,
        TurnOnAutoSend15Hz,
        TurnOnAutoSend25Hz,
        SetSpeed9600,
        SetSpeed19200,
        SetSpeed115200,
        SetFilter1,
        SetFilter2,
        SetFilter3,
        SetAddress0,
        SetRelativeAngle,
        SetAbsoluteAngle,
    };

    int get485Cmd(Cmds cmd, unsigned char *buf16, int addr = -1) const;

private:
    float m_crossAngle;
    float m_longAngle;

    char m_bufIn[64];
    int m_posIn;
    time_t m_lastReceivedTime;
    void checkBuf();

    int m_memSeq;
    bool m_isMem;
    float m_offsetCross;
    float m_offsetLong;
    float m_kCross;
    float m_kLong;
    bool m_isSwitchAxes;

    void checkMem();
    void setNewAngle(float fx, float fy);


    float encode485Angle(const unsigned char *val);

    void process232Buffer(const char *buf);
    void process485cmd(const unsigned char *data);

    int m_d485Address;

public:
    void setFullContext(int fullContext);
    const char *groupVar() const;

    int d485Address() const;
    void setD485Address(int d485Address);

protected:
    void rsReceive(const unsigned char *buf, int len, bool is485);
private:
    const char *m_groupVar;
};




#endif // SENSORDIGITALANGLE_H
