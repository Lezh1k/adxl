#include "rsprocessbus68.h"

static const int d485StartChar = 0x68;

RsProcessBus68::RsProcessBus68():
    BaseRsProcess(),
    m_posIn68(0),
    m_is485(true)
{

}

RsProcessBus68::~RsProcessBus68()
{

}



void RsProcessBus68::receiveChar(unsigned char ch, bool is485)
{
    m_is485 = is485;
    depack485Proto68(ch);
}

void RsProcessBus68::sendPrepared(const unsigned char *buf, int len) const
{
    if(buf[0]!=d485StartChar){
        unsigned char b[1];
        b[0] = d485StartChar;
        sendRaw(b,1);
        b[0] = len+1;
        sendRaw(b,1);
        sendRaw(buf,len);
        unsigned char sum = 0;
        for(int i=0;i<len;i++)
            sum += buf[i];
        b[0] = sum + len + 1;
        sendRaw(b,1);
    }else{
        sendRaw(buf,len);
    }
}

void RsProcessBus68::depack485Proto68(unsigned char sym)
{
    if(m_posIn68==0 && sym != d485StartChar){
        return;
    }
    if(m_posIn68 > static_cast<int>(sizeof(m_buf68)-10)){
        m_posIn68 = 0;
        return;
    }
    m_buf68[m_posIn68++] = sym;
    unsigned char *buf = reinterpret_cast<unsigned char*>(m_buf68);

    if(m_posIn68 > 3){
        unsigned int l = buf[1];
        if(l > sizeof(m_buf68)-4 || l<4){
            m_posIn68 = 0;
            return;
        }
        if(m_posIn68 == l+1){
            if(true){ //buf[2] == d485Address){
                unsigned char sum = 0;
                for(unsigned int i=1;i<l;i++){
                    sum += buf[i];
                }
                if(sum == buf[l]){
                    process485cmd68(buf);
                }
            }
            m_posIn68 = 0;
            return;
        }else{
            if(m_posIn68 > l+1){
                m_posIn68 = 0;
                return;
            }
        }
    }
}

void RsProcessBus68::process485cmd68(unsigned char *cmd)
{
    receiveToAll(cmd,cmd[1],m_is485);
}
