#include "Packet.h"
#include <inttypes.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <iostream>

constexpr int PACKET_BUFFER_SIZE = 296;

PacketInfo::PacketInfo() {}
PacketInfo::~PacketInfo() {}

void PacketInfo::push(uint8_t *receiveBuffer) {
    std::memcpy(pointerToFirstElement(), receiveBuffer, PACKET_BUFFER_SIZE);
}

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

float PacketInfo::getTyreSpeed(int index) {
    if (index >= 0 && index < 4) {
        return abs(3.6f * m_packet.tyresRadius[index] * m_packet.wheelRPS[index]);
    } else return 0.0f;
}

float PacketInfo::getTyreSlipRatio(int index) {
    float carSpeed = (m_packet.speed * 3.6);
    float tyreSpeed = getTyreSpeed(index);
    if (carSpeed != 0.0f) {
        return tyreSpeed / carSpeed;
    } else return 0.0f;
}

uint8_t PacketInfo::getFlag(int index) {
    SimulatorFlags flags = m_packet.flags;
    int16_t indexAdjusted = index - 1;
    if (index < 0 || index > 13) {
        return 0;
    }

    if (index == 0) {
        return (static_cast<int16_t>(flags) == 0) ? 1 : 0;
    } else {
        return (index >= 1 && static_cast<int16_t>(flags) & (1 << indexAdjusted)) ? 1 : 0;
    }
}

int32_t* PacketInfo::pointerToFirstElement(void) {
    return &m_packet.magic;
}