#include <WiFiUdp.h>
#include "GT7UDPParser.h"
#include "Salsa20.h"
#include <string>
#include <span>
#include <array>
#include <vector>

constexpr unsigned int localPort = 33740; 
constexpr unsigned int remotePort = 33739; 
constexpr char heartbeatMsg = 'A';
const std::string Key = "Simulator Interface Packet GT7 ver 0.0";

Packet packet;

union IntToBytes {
    uint32_t integer;
    uint8_t bytes[4];
};

std::array<uint8_t, 32> GT7_UDP_Parser::getAsciiBytes(const std::string& inputString) {
    std::array<uint8_t, 32> asciiBytes = {};
    for (size_t i = 0; i < inputString.size() && i < asciiBytes.size(); ++i) {
        asciiBytes[i] = static_cast<uint8_t>(inputString[i]);
    }
    return asciiBytes;
}

void GT7_UDP_Parser::begin(const IPAddress playstationIP) {
    Udp.begin(localPort);
    remoteIP = playstationIP;
    dKey = getAsciiBytes(Key);
}

void GT7_UDP_Parser::sendHeartbeat(void) {
    Udp.beginPacket(remoteIP, remotePort);
    Udp.write(heartbeatMsg);
    Udp.endPacket();
}

uint8_t GT7_UDP_Parser::getCurrentGearFromByte(void) {
    return packet.packetContent.gears & 0b00001111; // Extract the lower 4 bits for gears
}

// Function to extract suggested gear from the byte
uint8_t GT7_UDP_Parser::getSuggestedGearFromByte(void) {
    return packet.packetContent.gears >> 4; // Shift right by 4 bits to get the upper 4 bits for suggested gear
}


uint8_t GT7_UDP_Parser::getPowertrainType(void) {
    if (static_cast<uint8_t>(packet.packetContent.fuelCapacity) > 10) {
    return 0;
    } else {
        switch(static_cast<uint8_t>(packet.packetContent.fuelCapacity)) {
            case 0: return 1;
                break;
            case 5: return 2;
                break;
            default: return 255;
    }
    }
}

float GT7_UDP_Parser::getTyreSpeed(int index) {
    if (index >= 0 && index < 4) {
        return abs(3.6f * packet.packetContent.tyreRadius[index] * packet.packetContent.wheelRPS[index]);
    } else return 0.0f;
}

float GT7_UDP_Parser::getTyreSlipRatio(int index) {
    float carSpeed = (packet.packetContent.speed * 3.6);
    float tyreSpeed = getTyreSpeed(index);
    if (carSpeed != 0.0f) {
        return tyreSpeed / carSpeed;
    } else return 0.0f;
}

uint8_t GT7_UDP_Parser::getFlag(int index) {
    SimulatorFlags flags = packet.packetContent.flags;
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

Packet GT7_UDP_Parser::readData(void) {
    uint8_t recvBuffer[sizeof(packet.packetContent)];
    memset(recvBuffer, 0, sizeof(packet.packetContent));
    Udp.parsePacket();
    if (Udp.read(recvBuffer, sizeof(recvBuffer)) == sizeof(packet.packetContent)) {
    int iv1 = *reinterpret_cast<int*>(&recvBuffer[0x40]); // Seed IV is always located there
    int iv2 = iv1 ^ 0xDEADBEAF;
    IntToBytes iv1Bytes, iv2Bytes;
    iv1Bytes.integer = iv1;
    iv2Bytes.integer = iv2;

    // Construct the 8-byte initialization vector
     uint8_t iv[8] = {
        iv2Bytes.bytes[0], iv2Bytes.bytes[1], iv2Bytes.bytes[2], iv2Bytes.bytes[3],
        iv1Bytes.bytes[0], iv1Bytes.bytes[1], iv1Bytes.bytes[2], iv1Bytes.bytes[3]
    };

    ucstk::Salsa20 salsa20(dKey.data());
    salsa20.setIv(iv);

    std::vector<uint8_t> decryptedData(sizeof(recvBuffer));
    salsa20.processBytes((recvBuffer), decryptedData.data(), 0x128);
    memcpy(&packet.packetContent, decryptedData.data(), sizeof(packet.packetContent));
    } 
    return packet;
}