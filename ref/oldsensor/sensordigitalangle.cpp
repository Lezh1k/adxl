#include <stdint.h>
#ifndef DEBUG_TEST_TOOL
#include "RTC/rtc_independent.h"
#include "Memory/memorizer.h"
#include "Config/Configure_main.h"

#else
time_t kobusTime();
#endif

#include "sensordigitalangle.h"
#include <string.h>
#include <stdlib.h>

#ifdef QT_CORE_LIB
extern char angleEmulationPlatormString[64];
#define DEFAULT_EMULATOR_ANGLE angleEmulationPlatormString //"X=+05.12;Y=+10.67\r\n"
#undef CFG_DIGITAL_ANGLE_TYPE
#define CFG_DIGITAL_ANGLE_TYPE 232
#else
#undef DEFAULT_EMULATOR_ANGLE
#endif

#define DEBUG 0

#if DEBUG
#include "interface/Hi_level.h"
#endif

#ifndef Q_UNUSED
#define Q_UNUSED(a) ((void)(a))
#endif

/*
 * 485 digital angle format:
 * 0x68, len(except 0x68), address, cmd, [data], crc = sum(all except 0x68)
 *
 * CMD:
 *      0x04 - read angle  (ans: 0x84, XXX, YYY, rrr)
 *      0x05 (0-abs, 1-rel) - set zerro
 *      0x0B (0-2400,4800,9600,19200,38400,5-115200) set speed
 *      0x0C (0-no, 1-5Hz, 15,25,35,50, 6-100Hz) set autorate
 *      0x0F (address 0...EF) - set address  (FF broadcast)
 *      0x17 (1-fast,2-peak,3-smooth) - set filter
 *
 */





SensorDigitalAngle::SensorDigitalAngle(const char *groupVar_):
    m_crossAngle(0),
    m_longAngle(0),
    m_posIn(0),
    m_lastReceivedTime(0),
    m_memSeq(0),
    m_isMem(false),
    m_offsetCross(0),
    m_offsetLong(0),
    m_kCross(1),
    m_kLong(1),
    m_isSwitchAxes(false),
    m_d485Address(0),
    m_groupVar(groupVar_)
{
    memset(m_bufIn,0,sizeof(m_bufIn));
    //log_printf(MODULF,"x=%f,y=%f  ",m_crossAngle,m_longAngle);
}


float SensorDigitalAngle::crossAngle() const
{
    return m_crossAngle;
}

float SensorDigitalAngle::longAngle() const
{
    return m_longAngle;
}

void SensorDigitalAngle::process232Buffer(const char *buf)
{
    m_posIn = 0;
    int p = strlen(m_bufIn);

    for(;p<(int)sizeof(m_bufIn)-1;){
        char c = *buf++;
        if(!c)
            break;
        if(c=='\n' || c=='\r'){
            if(p){
                m_bufIn[p] = 0;
                checkBuf();
            }
            p = 0;
            continue;
        }
        m_bufIn[p++] = c;
    }
    m_bufIn[p] = 0;

    if(p>(int)sizeof(m_bufIn)-3) //сбрасываем буффер если мусор пришел
        m_bufIn[0] = 0;

}


void SensorDigitalAngle::process485cmd(const unsigned char *data)
{
    if(data[2]==2)// && data[2]!=0xFF)
        return;
    if(data[3]==0x84 && data[1]>=0x0D){
        // 0x68 0x0D adr 0x84 X X X Y Y Y r r r CRC
        float fx = encode485Angle(data+4);
        float fy = encode485Angle(data+7);

        setNewAngle(fx, fy);
    }
}
int SensorDigitalAngle::d485Address() const
{
    return m_d485Address;
}

void SensorDigitalAngle::setD485Address(int d485Address)
{
    m_d485Address = d485Address;
}


float SensorDigitalAngle::encode485Angle(const unsigned char *val)
{
    bool isMinus = val[0] & 0x10;
    int beforeDot = ((val[1] & 0xF0)>>4) * 10 + (val[1] & 0x0F);
    int afterDot = ((val[2] & 0xF0)>>4) * 10 + (val[2] & 0x0F);

    float res = beforeDot + (afterDot / 100.0);
    return isMinus ? -res : res;
}

#ifdef QT_CORE_LIB
#include "workwithrs.h"
#endif

bool SensorDigitalAngle::isAnglePresent()const
{
#if 0 && defined(DEFAULT_EMULATOR_ANGLE)
//#if CFG_DIGITAL_ANGLE_TYPE == 232
    WorkWithRs::instance()->debugSendToSensors(2,DEFAULT_EMULATOR_ANGLE,strlen(DEFAULT_EMULATOR_ANGLE));
    //process232Buffer(DEFAULT_EMULATOR_ANGLE);
//#else
    unsigned char k[]={0x68, 0x0D, 0x00, 0x84, 0x00, 0x20, 0x10, 0x10, 0x05, 0x25, 0x00, 0x00, 0x00, 0xFB};
    //process485cmd(k);
    WorkWithRs::instance()->debugSendToSensors(3,k,sizeof(k));
//#endif
#endif
    return m_lastReceivedTime > kobusTime() - 3;
}

int SensorDigitalAngle::get485Cmd(SensorDigitalAngle::Cmds cmd, unsigned char *buf16, int addr)const
{
    if(addr<0)
        addr = d485Address();
    const char *c=0;
    switch (cmd) {
    case SensorDigitalAngle::RequestAngle:          c = "\x68\x04\x00\x04\x08";     break;
    case SensorDigitalAngle::TurnOffAutoSend:       c = "\x68\x05\x00\x0C\x00\x11"; break;
    case SensorDigitalAngle::TurnOnAutoSend5Hz:     c = "\x68\x05\x00\x0C\x01\x12"; break;
    case SensorDigitalAngle::TurnOnAutoSend15Hz:    c = "\x68\x05\x00\x0C\x02\x13"; break;
    case SensorDigitalAngle::TurnOnAutoSend25Hz:    c = "\x68\x05\x00\x0C\x03\x14"; break;
    case SensorDigitalAngle::SetSpeed9600:          c = "\x68\x05\x00\x0B\x02\x12"; break;
    case SensorDigitalAngle::SetSpeed19200:         c = "\x68\x05\x00\x0B\x03\x13"; break;
    case SensorDigitalAngle::SetSpeed115200:        c = "\x68\x05\x00\x0B\x05\x15"; break;
    case SensorDigitalAngle::SetFilter1:            c = "\x68\x05\x00\x17\x01\x1D"; break;
    case SensorDigitalAngle::SetFilter2:            c = "\x68\x05\x00\x17\x02\x1E"; break;
    case SensorDigitalAngle::SetFilter3:            c = "\x68\x05\x00\x17\x03\x1F"; break;
    case SensorDigitalAngle::SetAddress0:           c = "\x68\x05\xFF\x0F\x00\x13"; break;
    case SensorDigitalAngle::SetRelativeAngle:      c = "\x68\x05\x00\x05\x01\x0B"; break;
    case SensorDigitalAngle::SetAbsoluteAngle:      c = "\x68\x05\x00\x05\x00\x0A"; break;
    }
    if(!c)
        return 0;
    int l = c[1]+1;
    memcpy(buf16,c,l);
    buf16[l-1]-=buf16[2];
    buf16[2]=addr;
    buf16[l-1]+=addr;
    return l;
}

void SensorDigitalAngle::setNewAngle(float fx, float fy)
{
    checkMem();
    receiveNext();

    if(m_isMem){
        float c,l;
        if(m_isSwitchAxes){
            l = fx;
            c = -fy;
        }else{
            c = -fx;
            l = fy;
        }

        m_longAngle = (l + m_offsetLong) * m_kLong;
        m_crossAngle = (c + m_offsetCross) * m_kCross;

    }else{

        m_longAngle = fx;
        m_crossAngle = -fy;

    }

    m_lastReceivedTime = kobusTime();
}

void SensorDigitalAngle::checkBuf()
{
    //X=+03.12;Y=+84.67
    char *p = m_bufIn;
    if(memcmp(p,"X=",2))
        return;
    p+=2;
    char *zpt = strchr(p,';');
    if(!zpt)
        return;
    *zpt++ = 0;
    if(memcmp(zpt,"Y=",2))
        return;
    zpt+=2;

    float fx = atof(p);
    float fy = atof(zpt);


    setNewAngle(fx, fy);

}

void SensorDigitalAngle::checkMem()
{
    #ifndef DEBUG_TEST_TOOL
    Memorizer *m = Memorizer::instance();
    if(m_memSeq != m->sequenceChange()){
        bool r = true;
        r &= m->readGroupValue(m_groupVar, angleVarOffsetLong, m_offsetLong);
        r &= m->readGroupValue(m_groupVar, angleVarOffsetCross, m_offsetCross);
        m_isMem = r;
        if(!m->readGroupValue(m_groupVar, angleVarCorrKLong,m_kLong))
            m_kLong = 1;
        if(m->readGroupValue(m_groupVar, angleVarCorrKCross,m_kCross))
            m_kCross = 1;
    }
    #endif
}


SensorDigitalAngle *sensorDigitalAngle = 0;


void SensorDigitalAngle::setFullContext(int context)
{
    m_d485Address = context;
}

void SensorDigitalAngle::rsReceive(const unsigned char *buf, int len, bool is485)
{
    if(is485){
        process485cmd(buf);
    }else{
        process232Buffer(reinterpret_cast<const char*>(buf));
    }
    Q_UNUSED(len);
}
const char *SensorDigitalAngle::groupVar() const
{
    return m_groupVar;
}

