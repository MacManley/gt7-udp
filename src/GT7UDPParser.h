#ifndef GT7UDPPARSER_H
#define GT7UDPPARSER_H

#include <inttypes.h>
#include <WiFiUdp.h>
#include <array>
#include <string>

#pragma pack(push, 1)

enum class SimulatorFlags : int16_t {
    None = 0,

    CarOnTrack = 1 << 0,

    Paused = 1 << 1,

    LoadingOrProcessing = 1 << 2,

    InGear = 1 << 3,

    HasTurbo = 1 << 4,

    RevLimiterBlinkAlertActive = 1 << 5,

    HandBrakeActive = 1 << 6,

    LightsActive = 1 << 7,

    HighBeamActive = 1 << 8,

    LowBeamActive = 1 << 9,

    ASMActive = 1 << 10,

    TCSActive = 1 << 11
};

struct GT7Packet { 
int32_t magic; // Magic, different value defines what game is being played 
float position[3]; // Position on Track in meters in each axis
float worldVelocity[3]; // Velocity in meters for each axis
float rotation[3]; // Rotation (Pitch/Yaw/Roll) (RANGE: -1 -> 1)
float orientationRelativeToNorth; // Orientation to North (RANGE: 1.0 (North) -> 0.0 (South))
float angularVelocity[3]; // Speed at which the car turns around axis in rad/s (RANGE: -1 -> 1)
float bodyHeight; // Body height
float EngineRPM; // Engine revolutions per minute
uint8_t iv[4]; // IV for Salsa20 encryption/decryption
float fuelLevel; // Fuel level of car in liters 
float fuelCapacity; // Max fuel capacity for current car (RANGE: 100 (most cars) -> 5 (karts) -> 0 (electric cars))  
float speed; // Speed in m/s
float boost; // Offset by +1 (EXAMPLE: 1.0 = 0 X 100kPa, 2.0 = 1 x 100kPa) // TODO apply -1 offset
float oilPressure; // Oil pressure in bars
float waterTemp; // Constantly 85
float oilTemp; // Constantly 110
float tyreTemp[4]; // Tyre temp for all 4 tires (FL -> FR -> RL -> RR)
int32_t packetId; // ID of packet
int16_t lapCount; // Lap count
int16_t totalLaps; // Laps to finish
int32_t bestLaptime; // Best lap time, defaults to -1 if not set
int32_t lastLaptime; // Previous lap time, defaults to -1 if not set
int32_t dayProgression; // Current time of day on track in ms
int16_t RaceStartPosition; // Position of the car before the start of the race, defaults to -1 after race start
int16_t preRaceNumCars; // Number of cars before the race start, defaults to -1 after start of the race
int16_t minAlertRPM; // Minimum RPM that the rev limiter displays an alert
int16_t maxAlertRPM; // Maximum RPM that the rev limiter displays an alert
int16_t calcMaxSpeed; // Highest possible speed achievable of the current transmission settings
SimulatorFlags flags; // Packet flags // TODO: Get working
uint8_t gears; // First 4 bits: Current Gear, Last 4 bits: Suggested Gear, see getCurrentGearFromByte and getSuggestedGearFromByte
uint8_t throttle; // Throttle (RANGE: 0 -> 255)
uint8_t brake; // Brake (RANGE: 0 -> 255)
uint8_t PADDING; // Padding byte
float roadPlane[3]; // Banking of the road 
float roadPlaneDistance; // Distance above or below the plane, e.g a dip in the road is negative, hill is positive.
float wheelRPS[4]; // Revolutions per second of tyres in rads
float tyreRadius[4]; // Radius of the tyre in meters
float suspHeight[4]; // Suspension height of the car
uint32_t UNKNOWNFLOATS[8]; // Unknown float
float clutch; // Clutch (RANGE: 0.0 -> 1.0)
float clutchEngagement; // Clutch Engangement (RANGE: 0.0 -> 1.0)
float RPMFromClutchToGearbox; // Pretty much same as engine RPM, is 0 when clutch is depressed
float transmissionTopSpeed; // Top speed as gear ratio value
float gearRatios[8]; // Gear ratios of the car up to 8
int32_t carCode; // This value may be overriden if using a car with more then 9 gears
//
};

struct Packet {
    GT7Packet packetContent;
};

#pragma pack(pop)

class GT7_UDP_Parser {
    public:
		void begin(const IPAddress playstationIP);
		void sendHeartbeat();
        uint8_t getFlag(int index);
        uint8_t getCurrentGearFromByte(void);
        uint8_t getSuggestedGearFromByte(void);
        uint8_t getPowertrainType(void);
        float getTyreSpeed(int index);
        float getTyreSlipRatio(int index);
        Packet readData();
    private: 
        WiFiUDP Udp;
        IPAddress remoteIP;
        std::array<uint8_t, 32> dKey;
        std::array<uint8_t, 32> getAsciiBytes(const std::string& inputString);
};

#endif