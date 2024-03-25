#include "Packet.h"
#include <inttypes.h>
#include <cstdio>
#include <cstring>

const int PACKET_BUFFER_SIZE = 296;

using namespace std;

PacketInfo::PacketInfo() {}
PacketInfo::~PacketInfo() {}

void PacketInfo::push(uint8_t *receiveBuffer) {
    std::memcpy(pointerToFirstElement(), receiveBuffer, PACKET_BUFFER_SIZE);
};

uint8_t PacketInfo::getCurrentGearFromByte(void) {
    return m_packet.gears & 0b00001111; // Extract the lower 4 bits for gears
}

// Function to extract suggested gear from the byte
uint8_t PacketInfo::getSuggestedGearFromByte(void) {
    return m_packet.gears >> 4; // Shift right by 4 bits to get the upper 4 bits for suggested gear
}


uint8_t PacketInfo::getPowertrainType(void) {
    if (static_cast<uint8_t>(m_packet.fuelCapacity) > 10) {
    return 0;
    } else {
        switch(static_cast<uint8_t>(m_packet.fuelCapacity)) {
            case 0: return 1;
                break;
            case 5: return 2;
                break;
            default: return 255;
    }
    }
}

int32_t* PacketInfo::pointerToFirstElement(void)
{
    return &m_packet.magic;
}