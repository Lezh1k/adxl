#ifndef RSPROCESSBUS68_H
#define RSPROCESSBUS68_H

#include "basersprocess.h"

class RsProcessBus68:
        public BaseRsProcess
{
public:
    RsProcessBus68();
    ~RsProcessBus68();

public:
    void receiveChar(unsigned char ch, bool is485);

protected:
    void sendPrepared(const unsigned char *buf, int len)const;
private:
    char m_buf68[64];
    unsigned int m_posIn68;
    bool m_is485;

    void depack485Proto68(unsigned char sym);
    void process485cmd68(unsigned char *cmd);
};

#endif // RSPROCESSBUS68_H
