# Gran Turismo 7 UDP | Library for use on ESP 32 / ESP8266 devices
**Data Output from the Gran Turismo 7 Game**

This program captures and parses packets that are sent by UDP from the Gran Turismo 7 game on PS4/PS5. This library is written specifically for usage on the ESP32 and ESP8266.

# Usage:
```C
#include "GT7UDPParser.h"
GT7_UDP_Parser* parser;

void loop()
{
    parser = new GT7_UDP_Parser();
    parser->push(*buffer);
}

```

### Packet

The following data types are used in the structure:

| Type   | Description             |
| ------ | ----------------------- |
| uint8  | Unsigned 8-bit integer  |
| int8   | Signed 8-bit integer    |
| uint16 | Unsigned 16-bit integer |
| int16  | Signed 16-bit integer   |
| uint32 | Unsigned 32-bit integer |
| float  | Floating point (32-bit) |
| uint64 | Unsigned 64-bit integer |

There is a singular encrypted packet sent out by GT7. The packet is encrypted in the Salsa20 stream cipher. The packets will only be sent from the console if there is a heartbeat sent from a device (in our case an ESP8266/ESP32). Once the console receives a heartbeat, it then establishes a connection with the ESP8266/ESP32 and sends the data to the IP address that was used to send the heartbeat. The console will expect a heartbeat every 100 packets (around 1.6 seconds) or else the connection will cease.

## The Packet: 

This singular packet details many different telemetry points such as positioning, car telemetry, laptimes and more.

Size: 296 bytes

Frequency: 60Hz (60 times a second)

```c#
struct Packet {
int32 magic; // Magic, different value defines what game is being played
float position[3]; // Position on Track in meters in each axis
float worldVelocity[3]; // Velocity in meters for each axis
float rotation[3]; // Rotation (Pitch/Yaw/Roll) (RANGE: -1 -> 1)
float orientationRelativeToNorth; // Orientation to North (RANGE: 1.0 (North) -> 0.0 (South))
float angularVelocity[3]; // Speed at which the car turns around axis in rad/s (RANGE: -1 -> 1)
float bodyHeight; // Body height
float EngineRPM; // Engine revolutions per minute
char iv1; // IV for Salsa20 encryption/decryption
char iv2; // IV for Salsa20 encryption/decryption
char iv3; // IV for Salsa20 encryption/decryption
char iv4; // IV for Salsa20 encryption/decryption
float fuelLevel; // Fuel level of car in liters 
float fuelCapacity; // Max fuel capacity for current car (RANGE: 100 (most cars) -> 5 (karts) -> 0 (electric cars))  
float speed; // Speed in m/s
float boost; // Offset by +1 (EXAMPLE: 1.0 = 0 X 100kPa, 2.0 = 1 x 100kPa)
float oilPressure; // Oil pressure in bars
float waterTemp; // Always 85
float oilTemp; // Always 110
float tyresTemp[4]; // Tyre temp for all 4 tires (FL -> FR -> RL -> RR)
int32 packetId; // ID of packet
int16 lapCount; // Lap count
int16 totalLaps; // Laps to finish
int32 bestLaptime; // Best lap time, defaults to -1 if not set
int32 lastLaptime; // Previous lap time, defaults to -1 if not set
int32 dayProgression; // Current time of day on track in ms
int16 RaceStartPosition; // Position of the car before the start of the race, defaults to -1 after race start
int16 preRaceNumCars; // Number of cars before the race start, defaults to -1 after start of the race
int16 minAlertRPM; // Minimum RPM that the rev limiter displays an alert
int16 maxAlertRPM; // Maximum RPM that the rev limiter displays an alert
int16 calcMaxSpeed; // Highest possible speed achievable of the current transmission settings
SimulatorFlags flags; // Packet flags
uint8 gears; // First 4 bits: Current Gear, Last 4 bits: Suggested Gear, see getCurrentGearFromByte and getSuggestedGearFromByte
uint8 throttle; // Throttle (RANGE: 0 -> 255)
uint8 brake; // Brake (RANGE: 0 -> 255)
uint8 UNKNOWNBYTE1; // Padding byte
float roadPlane[3]; // Banking of the road
float roadPlaneDistance; // Distance above or below the plane, e.g a dip in the road is negative, hill is positive.
float wheelRPS[4]; // Revolutions per second of tyres in rads
float tyresRadius[4]; // Radius of the tyre in meters
float suspHeight[4]; // Suspension height of the car
uint32 UNKNOWNFLOAT2; // Unknown float
uint32 UNKNOWNFLOAT3; // Unknown float
uint32 UNKNOWNFLOAT4; // Unknown float
uint32 UNKNOWNFLOAT5; // Unknown float
uint32 UNKNOWNFLOAT6; // Unknown float
uint32 UNKNOWNFLOAT7; // Unknown float
uint32 UNKNOWNFLOAT8; // Unknown float
uint32 UNKNOWNFLOAT9; // Unknown float
float clutch; // Clutch (RANGE: 0.0 -> 1.0)
float clutchEngagement; // Clutch Engangement (RANGE: 0.0 -> 1.0)
float RPMFromClutchToGearbox; // Pretty much same as engine RPM, is 0 when clutch is depressed
float transmissionTopSpeed; // Top speed as gear ratio value
float gearRatios[8]; // Gear ratios of the car up to 8
int32 carCode; // This value may be overriden if using a car with more then 9 gears
};

struct PacketInfo
{
Packet m_packet;
};
```

> Note: SimulatorFlags is currently not accessible via the library. This is a Work In Progress

## Additional Functions

These are the current additional functions needed to retrieve additional information not immediately present in the packet

```c#
uint8 getCurrentGearFromByte() // Using gears
uint8 getSuggestedGearFromByte() // Using gears
uint8 getPowertrainType() // Using fuelCapacity, 0: Combustion Engine, 1: Electric, 2: Karts
```

Here is how you can use them in your code:

```C
#include "GT7UDPParser.h"
GT7_UDP_Parser* parser;

void loop()
{
    parser = new GT7_UDP_Parser();
    parser->push(*buffer);
    uint8_t extractedGears = parser->packetInfo()->getCurrentGearFromByte();
    uint8_t extractedSuggestedGear = parser->packetInfo()->getSuggestedGearFromByte();
    uint8_t extractedPowertrain = parser->packetInfo()->getPowertrainType();
}

```

### Encryption 

The packet is encrypted in the Salsa20 cipher stream with a 32 byte key and 8 byte nonce.

## ***Key***

``` 
Simulator Interface Packet GT7 ver 0.0
```

This key is 39 bytes long. Each character in the string has to be converted into a byte slice of length 32, so the first 32 characters are used. Each character converted of the string will be represented by its corresponding ASCII value.

## ***Nonce***

The beginnings of the 8 byte nonce can be located at position \[0x40:0x44]. 4 bytes are extracted from the buffer. The extracted value is interpreted as a 32-bit integer, denoted as `iv1`. This value also undergoes an XOR operation with the constant `0xDEADBEAF`, producing a new integer value called `iv2`. These byte slices are then combined into the full 8 byte nonce, with the first 4 bytes initialized as `iv2`, and the last 4 bytes as `iv1`.

### Credits

Credit to [everard](https://github.com/everard/Salsa20) for the C++11 implementation of the stream cipher Salsa20

Credit to [Nenkai](https://github.com/Nenkai/PDTools) for help with IV setup and their work on GT7 UDP Capabilities

Thanks to members of GTPlanet for their findings on this hidden gem in GT7, find the thread [here](https://www.gtplanet.net/forum/threads/gt7-is-compatible-with-motion-rig.410728/)

