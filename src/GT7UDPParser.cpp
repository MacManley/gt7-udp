#include <stdio.h>
#include <string>
#include <span>
#include <array>
#include <vector>
#include "GT7UDPParser.h"
#include "Packet.h"
#include "Salsa20.h"
#include <iostream>

const uint8_t* GetAsciiBytes(const std::string& inputString, size_t numBytes) {
    // Allocate memory for the ASCII bytes
    uint8_t* asciiBytes = new uint8_t[numBytes];

    // Copy ASCII bytes from inputString to asciiBytes
    for (size_t i = 0; i < inputString.size() && i < numBytes; ++i) {
        asciiBytes[i] = static_cast<uint8_t>(inputString[i]);
    }

    return asciiBytes;
}

 bool checkMagic(uint8_t* decryptedData) {
    // Define the expected header as an array of bytes
    uint8_t magic[] = { 0x47, 0x37, 0x53, 0x30 }; // Assuming null-terminated string

    // Compare the first 5 bytes of receiveBuffer with the expectedHeader
    return memcmp(decryptedData, magic, 4) == 0;
}

GT7_UDP_Parser::GT7_UDP_Parser()
{
    packetInfo_ = new PacketInfo();
}

GT7_UDP_Parser::~GT7_UDP_Parser()
{
    delete packetInfo_;
}

void GT7_UDP_Parser::push(uint8_t * receiveBuffer)
{
    int iv1 = *reinterpret_cast<int*>(&receiveBuffer[0x40]); // Seed IV is always located there
    int iv2 = iv1 ^ 0xDEADBEAF;
        const uint8_t iv[8] = {
        static_cast<uint8_t>(iv2 & 0xFF), static_cast<uint8_t>((iv2 >> 8) & 0xFF), 
        static_cast<uint8_t>((iv2 >> 16) & 0xFF), static_cast<uint8_t>((iv2 >> 24) & 0xFF),
        static_cast<uint8_t>(iv1 & 0xFF), static_cast<uint8_t>((iv1 >> 8) & 0xFF),
        static_cast<uint8_t>((iv1 >> 16) & 0xFF), static_cast<uint8_t>((iv1 >> 24) & 0xFF)
};
        std::string Key = "Simulator Interface Packet GT7 ver 0.0";
        const uint8_t* asciiBytes = GetAsciiBytes(Key, 32);
        ucstk::Salsa20 salsa20(asciiBytes);
        salsa20.setIv(iv);

        std::vector<uint8_t> decryptedData(0x128);
        salsa20.processBytes((receiveBuffer), decryptedData.data(), 0x128);

        bool checkmagic = checkMagic(decryptedData.data());

        if (checkMagic) {
            // Pass the decrypted data to packetInfo_->push()
            packetInfo_->push(decryptedData.data());
        }
    delete[] asciiBytes;
    
}

PacketInfo* GT7_UDP_Parser::packetInfo(void)
{
    return packetInfo_;
}