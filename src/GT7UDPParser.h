#ifndef GT7UDPPARSER_H
#define GT7UDPPARSER_H
#include "Packet.h"

class GT7_UDP_Parser {
public:
    GT7_UDP_Parser();
    virtual ~GT7_UDP_Parser();
    void push(uint8_t * receiveBuffer);
    PacketInfo* packetInfo(void);

private:
    PacketInfo* packetInfo_;
};

#endif