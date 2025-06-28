# Gran Turismo 7 UDP | Library for use on ESP 32 / ESP8266 devices
**Data Output from the Gran Turismo 7 Game**

This program captures and parses packets that are sent by UDP from the Gran Turismo 7 game on PS4/PS5. This library is written specifically for usage on the ESP32 and ESP8266.

# Usage:
```c++
#include "GT7UDPParser.h"
GT7_UDP_Parser gt7Telem;
Packet packetContent;
IPAddress playstationIP;
char packetVersion;

void setup()
{
    gt7Telem.begin(playstationIP, packetVersion);
    gt7Telem.sendHeartbeat();
}

void loop()
{
    packetContent = gt7Telem.read();
}
```

### Packet

The following data types are used in the structure:

| Type   | Description             |
| ------ | ----------------------- |
| uint8  | Unsigned 8-bit integer  |
| int16  | Signed 16-bit integer   |
| uint32 | Unsigned 32-bit integer |
| int32  | Signed 32-bit integer   |
| float  | Floating point (32-bit) |

There is an encrypted packet sent out by GT7. The packet is encrypted in the Salsa20 stream cipher. The packets will only be sent from the console if there is a heartbeat sent from a device (in our case an ESP8266/ESP32). Once the console receives a heartbeat, it then establishes a connection with the ESP8266/ESP32 and sends the data to the IP address that was used to send the heartbeat. The console will expect a heartbeat every 100 packets (around 1.6 seconds) or else the connection will cease.

## Packet Types

Since Update 1.42, two new packet versions/types have been added to the game, known as the "B" and "\~" packets, alongside the original "A" packet. These new packets can be accessed by changing the "A" character sent via the heartbeat to either "B" or "\~". 

## Packet "A": 

This base packet details many different telemetry points such as positioning, car telemetry, laptimes and more.

Size: 296 bytes

Frequency: 60Hz (60 times a second)

```c++
struct PacketA {
int32_t magic; // Magic, different value defines what game is being played
float position[3]; // Position on Track in meters in each axis
float worldVelocity[3]; // Velocity in meters for each axis
float rotation[3]; // Rotation (Pitch/Yaw/Roll) (RANGE: -1 -> 1)
float orientationRelativeToNorth; // Orientation to North (RANGE: 1.0 (North) -> 0.0 (South))
float angularVelocity[3]; // Speed at which the car turns around axis in rad/s (RANGE: -1 -> 1)
float bodyHeight; // Body height
float EngineRPM; // Engine revolutions per minute
uint8 iv[4]; // IV for Salsa20 encryption/decryption
float fuelLevel; // Fuel level of car in liters 
float fuelCapacity; // Max fuel capacity for current car (RANGE: 100 (most cars) -> 5 (karts) -> 0 (electric cars))  
float speed; // Speed in m/s
float boost; // Offset by +1 (EXAMPLE: 1.0 = 0 X 100kPa, 2.0 = 1 x 100kPa)
float oilPressure; // Oil pressure in bars
float waterTemp; // Always 85
float oilTemp; // Always 110
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
SimulatorFlags flags; // Packet flags
uint8_t gears; // First 4 bits: Current Gear, Last 4 bits: Suggested Gear, see appendices (getCurrentGearFromByte and getSuggestedGearFromByte)
uint8_t throttle; // Throttle (RANGE: 0 -> 255)
uint8_t brake; // Brake (RANGE: 0 -> 255)
uint8_t UNKNOWNBYTE1; // Padding byte
float roadPlane[3]; // Banking of the road
float roadPlaneDistance; // Distance above or below the plane, e.g a dip in the road is negative, hill is positive.
float wheelRPS[4]; // Revolutions per second of tyres in rads
float tyreRadius[4]; // Radius of the tyre in meters
float suspHeight[4]; // Suspension height of the car
float UNKNOWNFLOATS[8]; // Unknown floats
float clutch; // Clutch (RANGE: 0.0 -> 1.0)
float clutchEngagement; // Clutch Engangement (RANGE: 0.0 -> 1.0)
float RPMFromClutchToGearbox; // Pretty much same as engine RPM, is 0 when clutch is depressed
float transmissionTopSpeed; // Top speed as gear ratio value
float gearRatios[8]; // Gear ratios of the car up to 8
int32_t carCode; // This value may be overriden if using a car with more then 9 gears, see appendices
};
```

## Packet "B":

Packet B features the same structure as Packet A, excluding 5 additional floats.

Size: 316 bytes

Frequency: 60Hz (not available in Sport Mode)

```c++
//...
struct PacketB : public PacketA {
float wheelRotation; // Calculates the wheel rotation in radians
float UNKNOWNFLOAT10; // Unknown float
float sway; // X axis acceleration
float heave; // Y axis acceleration
float surge; // Z axis acceleration
};
```

## Packet "\~":

Packet ~ includes all data from both Packet A and Packet B, while adding various miscellanous datapoints such as active aero vectors.

Size: 344 Bytes

Frequency: 60Hz (not available in Replays)

```c++
//...
struct PacketC : public PacketB {
uint8_t throttleFiltered; // Filtered Throttle Output
uint8_t brakeFiltered; // Filtered Brake Output
uint8_t UNKNOWNUINT81; // Unknown unsigned 8 bit integer
uint8_t UNKNOWNUINT82; // Unknown unsigned 8 bit integer
float leftFlapDeflection; // Deflection of the active aero on the left side of the car. (RANGE: -1.0 -> 1.0)
float rightFlapDeflection; // Deflection of the active aero on the right side of the car. (RANGE: -1.0 -> 1.0)
float leftRearFlapMode; // Mode of the active aero on the left side of the car (RANGE: -1.0 -> 1.0)
float rightRearFlapMode; // Mode of the active aero on the right side of the car (RANGE: -1.0 -> 1.0)
float energyRecovery; // Energy being recovered to the battery
float UNKNOWNFLOAT11; // Unknown float
}
```

> Note: Unlike the unknown bytes and floats of packet A which are assumed to be padding, Packet "B" and "\~" are still under ongoing research to determine the function of the unknown integers and floats. The packet structure of both is subject to change upon any findings.

## Encryption 

The packet is encrypted in the Salsa20 cipher stream with a 32 byte key and 8 byte nonce.

### ***Key***

```
Simulator Interface Packet GT7 ver 0.0
```

This key is 39 bytes long. Each character in the string has to be converted into a byte slice of length 32, so the first 32 characters are used. Each character converted of the string will be represented by its corresponding ASCII value.

### ***Nonce***

The beginnings of the 8 byte nonce can be located at position \[0x40:0x44]. 4 bytes are extracted from the buffer. The extracted value is interpreted as a 32-bit integer, denoted as `iv1`. This value also undergoes an XOR operation with the constant `0xDEADBEAF`, `0xDEADBEEF` or `0x55FABB4F` depending on the packet version, producing a new integer value called `iv2`. These byte slices are then combined into the full 8 byte nonce, with the first 4 bytes initialized as `iv2`, and the last 4 bytes as `iv1`.

# **Appendices**

## Additional Functions

These are the current additional functions needed to retrieve additional information not immediately present in the packet

```c++
uint8_t getCurrentGearFromByte() // Get the currently selected gear, Using gears
uint8_t getSuggestedGearFromByte() // Get the currently suggested gear, Using gears (Will be 15 if no gear is currently suggested)
uint8_t getPowertrainType() // Get the powertrain type, Using fuelCapacity, 0: Combustion Engine, 1: Electric, 2: Karts
float getTyreSpeed() // Get linear tyre speed, using tyreRPS and tyreRadius
float getTyreSlipRatio() // Get the tyre slip ratio, using speed and tyreSpeed
```

Here is how you can use them in your program:

```c++
#include "GT7UDPParser.h"
GT7_UDP_Parser gt7Telem;
Packet packetContent;

void setup()
{
    gt7Telem.begin();
    gt7Telem.sendHeartbeat();
}

void loop()
{
    packetContent = gt7Telem.read();
    uint8_t currentGear = gt7Telem.getCurrentGearFromByte();
}
```


## Simulator Flags


There are bit flags sent out in the packet that detail game and car state. flags is a signed 16 bit integer.

|ID  |Name                  |Operation|
|----|----------------------|---------------|
|0   |None (No Active Flags)|flags == 0     | 
|1   |Car On Track          |flags & 1 << 0 |
|2   |Paused                |flags & 1 << 1 |
|3   |Loading Or Processing |flags & 1 << 2 |
|4   |In Gear               |flags & 1 << 3 |
|5   |Has Turbo             |flags & 1 << 4 |
|6   |Rev Limit Alert Active|flags & 1 << 5 |
|7   |Handbrake Active      |flags & 1 << 6 |
|8   |Lights Active         |flags & 1 << 7 |
|9   |High Beams Active     |flags & 1 << 8 |
|10  |Low Beams Active      |flags & 1 << 9 |
|11  |ASM Active            |flags & 1 << 10|
|12  |TCS Active            |flags & 1 << 11|

These bit flags can be accessed using the getFlag() function as shown below:

```C++
#include "GT7UDPParser.h"
GT7_UDP_Parser gt7Telem;
Packet packetContent;

void loop()
{
    packetContent = gt7Telem.read();
    uint8_t TCSActive = gt7Telem.getFlag(12);
    if (TCSActive) {
    Serial.println("TCS: Active");
    } else {
    Serial.println("TCS: Not Active");
    }
}
```


## Car and Manufacturer IDs


|ID  |Name                                      |Manufacturer|
|----|-----------------------------------------------|-----|
|24  |180SX Type X '96                               |28   |
|31  |Camaro Z28 '69                                 |7    |
|36  |Chevelle SS 454 Sport Coupé                    |7    |
|41  |Corvette Stingray (C3) '69                     |7    |
|48  |Fairlady 240ZG (HS30) '71                      |28   |
|63  |Corolla Levin 1600GT APEX (AE86) '83           |43   |
|78  |Silvia K's Aero (S14) '96                      |28   |
|82  |Supra RZ '97                                   |43   |
|104 |Sileighty '98                                  |28   |
|105 |205 Turbo 16 Evolution 2 '86                   |32   |
|116 |GT-One (TS020) '99                             |43   |
|135 |S800 '66                                       |15   |
|137 |Beat '91                                       |15   |
|145 |Copen '02                                      |10   |
|173 |R5 Turbo '80                                   |34   |
|187 |Tuscan Speed 6 '00                             |44   |
|201 |Eunos Roadster (NA) '89                        |21   |
|203 |Integra Type R (DC2) '98                       |15   |
|204 |Civic Type R (EK) '98                          |15   |
|205 |RX-7 Spirit R Type A (FD) '02                  |21   |
|207 |MR2 GT-S '97                                   |43   |
|210 |R34 GT-R V-spec II Nur '02                     |28   |
|211 |Lancer Evolution V GSR '98                     |25   |
|216 |McLaren F1 GTR Race Car '97                    |6    |
|293 |NSX Type R '92                                 |15   |
|296 |787B '91                                       |21   |
|301 |Lancer Evolution IV GSR '96                    |25   |
|315 |Cobra 427 '66                                  |36   |
|334 |Clio V6 24V '00                                |34   |
|345 |Sprinter Trueno 1600GT APEX (S.Shigeno Version)|43   |
|365 |155 2.5 V6 TI '93                              |3    |
|374 |RX-7 GT-X (FC) '90                             |21   |
|379 |Impreza Coupe WRX Type R STi Ver.VI '99        |38   |
|387 |300 SL Coupe '54                               |22   |
|396 |NSX Type R '02                                 |15   |
|451 |Impreza 22B-STi '98                            |38   |
|485 |GT-R GT500 '99                                 |28   |
|489 |R33 GT-R V-spec '97                            |28   |
|514 |S2000 '99                                      |15   |
|533 |Stratos '73                                    |18   |
|543 |XJ220 '92                                      |17   |
|575 |190 E 2.5-16 Evolution II '91                  |22   |
|604 |2000GT '67                                     |43   |
|665 |Superbird '70                                  |55   |
|688 |Integra Type R (DC2) '95                       |15   |
|709 |Fairlady Z 300ZX TT 2seater '89                |28   |
|773 |R32 GT-R V-spec II '94                         |28   |
|779 |Cappuccino (EA11R) '91                         |39   |
|781 |Celica GT-FOUR Rally Car (ST205) '95           |43   |
|799 |Lancer Evolution VIII MR GSR '04               |25   |
|808 |V6 Escudo Pikes Peak Special spec.98           |39   |
|810 |Sprinter Trueno 1600GT APEX (AE86) '83         |43   |
|818 |Corvette (C2) '63                              |7    |
|821 |Civic Type R (EK) '97                          |15   |
|829 |Delta HF Integrale Evoluzione '91              |18   |
|836 |Skyline 2000GT-R (KPGC110) '73                 |28   |
|837 |Skyline Hard Top 2000GT-R (KPGC10) '70         |28   |
|843 |Supra 3.0GT Turbo A '88                        |43   |
|919 |Silvia Q's (S13) '88                           |28   |
|931 |Lancer Evolution III GSR '95                   |25   |
|942 |Corvette ZR-1 (C4) '89                         |7    |
|954 |R92CP '92                                      |28   |
|959 |TT Coupe 3.2 quattro '03                       |5    |
|998 |Sauber Mercedes C9 '89                         |22   |
|1027|DeLorean S2 '04                                |65   |
|1040|Ford GT LM Race Car Spec II                    |13   |
|1044|Sports 800 '65                                 |43   |
|1067|XJR-9 '88                                      |17   |
|1069|2J '70                                         |93   |
|1365|R8 4.2 '07                                     |5    |
|1370|MINI Cooper S '05                              |51   |
|1373|Viper GTS '02                                  |11   |
|1378|F430 '06                                       |110  |
|1384|Fairlady Z Version S (Z33) '07                 |28   |
|1385|Swift Sport '07                                |39   |
|1399|M3 '07                                         |6    |
|1402|Viper SRT10 Coupe '06                          |11   |
|1409|F40 '92                                        |110  |
|1410|512 BB '76                                     |110  |
|1425|Ford GT LM Spec II Test Car                    |13   |
|1426|Ford GT '06                                    |13   |
|1431|RE Amemiya FD3S RX-7                           |87   |
|1433|Amuse S2000 GT1 Turbo                          |59   |
|1448|SILVIA spec-R Aero (S15) '02                   |28   |
|1458|Fairlady Z (Z34) '08                           |28   |
|1461|Silvia K's Dia Selection (S13) '90             |28   |
|1466|GT-R GT500 '08                                 |28   |
|1470|Supra GT500 '97                                |43   |
|1474|Enzo Ferrari '02                               |110  |
|1480|Corvette ZR1 (C6) '09                          |7    |
|1481|Countach 25th Anniversary '88                  |112  |
|1484|Countach LP400 '74                             |112  |
|1504|458 Italia '09                                 |110  |
|1506|Gallardo LP 560-4 '08                          |112  |
|1507|SLS AMG '10                                    |153  |
|1508|Lancer Evolution VI GSR T.M. SCP '99           |25   |
|1510|NSX GT500 '08                                  |15   |
|1516|SC430 GT500 '08                                |50   |
|1523|500 F '68                                      |12   |
|1527|500 1.2 8V Lounge SS '08                       |12   |
|1528|SLR McLaren '09                                |22   |
|1536|Zonda R '09                                    |30   |
|1537|Prius G '09                                    |43   |
|1539|GranTurismo S '08                              |116  |
|1540|McLaren F1 '94                                 |117  |
|1541|TTS Coupe '09                                  |5    |
|1542|Corvette Convertible (C3) '69                  |7    |
|1543|Challenger R/T '70                             |11   |
|1544|430 Scuderia '07                               |110  |
|1545|Murcielago LP 640 '09                          |112  |
|1549|Amuse NISMO 380RS Super Leggera                |59   |
|1551|330 P4 '67                                     |110  |
|1553|XJ13 '66                                       |17   |
|1562|LFA '10                                        |50   |
|1563|Megane Trophy '11                              |34   |
|1565|Mark IV Race Car '67                           |13   |
|1578|8C Competizione '08                            |3    |
|1581|GT by Citroen Road Car                         |9    |
|1582|Miura P400 Bertone Prototype '67               |112  |
|1645|GIULIA TZ2 carrozzata da ZAGATO '65            |3    |
|1646|908 HDi FAP '10                                |32   |
|1671|Sambabus Typ 2 '62                             |46   |
|1689|Civic Type R (EK) Touring Car                  |15   |
|1722|MP4-12C '10                                    |117  |
|1729|Mustang Mach 1 '71                             |13   |
|1746|Roadster Touring Car                           |21   |
|1770|Aventador LP 700-4 '11                         |112  |
|1773|Scirocco R '10                                 |46   |
|1778|Volkswagen 1200 '66                            |46   |
|1796|A110 '72                                       |86   |
|1797|SLS AMG GT3 '11                                |153  |
|1893|Z8 '01                                         |6    |
|1895|Dino 246 GT '71                                |110  |
|1896|Model S Signature Performance '12              |119  |
|1898|One-77 '11                                     |4    |
|1900|XNR Ghia Roadster '60                          |55   |
|1902|Z4 GT3 '11                                     |6    |
|1904|M3 GT '11                                      |6    |
|1905|GT-R NISMO GT3 '13                             |28   |
|1907|X-BOW R '12                                    |121  |
|1916|Corvette C7 '14                                |7    |
|1925|G.T.350 '65                                    |36   |
|1926|Cobra Daytona Coupe '64                        |36   |
|1927|Sport quattro S1 Pikes Peak '87                |5    |
|1931|250 GT Berlinetta passo corto '61              |110  |
|1932|1500 Biposto Bertone B.A.T 1 '52               |125  |
|1933|MiTo '09                                       |3    |
|1935|GT40 Mark I '66                                |13   |
|1956|Viper GTS '13                                  |11   |
|1965|R18 TDI '11                                    |5    |
|1973|Abarth 500 '09                                 |125  |
|1975|RX500 '70                                      |21   |
|1984|McLaren F1 GTR - BMW '95                       |117  |
|1985|Firebird Trans Am '78                          |52   |
|1986|R8 Gordini '66                                 |34   |
|1987|Megane R.S. Trophy '11                         |34   |
|1990|Diablo GT '00                                  |112  |
|2010|250 GTO '62                                    |110  |
|2011|500 Mondial Pinin Farina Coupe '54             |110  |
|2017|GTO '84                                        |110  |
|2018|365 GTB4 '71                                   |110  |
|2026|Aqua S '11                                     |43   |
|2049|Veyron 16.4 '13                                |113  |
|2050|Huayra '13                                     |30   |
|2051|Genesis Coupe 3.8 '13                          |16   |
|2055|Mercedes-Benz AMG VGT                          |153  |
|2059|Corvette Stingray Racer Concept '59            |7    |
|2060|Racing Kart 125 Shifter                        |33   |
|2074|M4 '14                                         |6    |
|2076|Mercedes-Benz AMG VGT Racing Series            |153  |
|2077|Red Bull X2014 Standard                        |33   |
|2078|Red Bull X2014 Junior                          |33   |
|2080|FT-1                                           |43   |
|2087|BMW VGT                                        |6    |
|2095|Concept XR-PHEV EVOLUTION VGT                  |25   |
|2098|GTI Roadster VGT                               |46   |
|2099|VIZIV GT VGT                                   |38   |
|2101|TS030 Hybrid '12                               |43   |
|2103|DP-100 VGT                                     |4    |
|2106|FT-1 VGT                                       |43   |
|2107|Chaparral 2X VGT                               |93   |
|2108|SRT Tomahawk X VGT                             |11   |
|2109|MINI Clubman VGT                               |51   |
|2110|SRT Tomahawk GTS-R VGT                         |11   |
|2111|SRT Tomahawk S VGT                             |11   |
|2112|Alpine VGT                                     |86   |
|2113|PEUGEOT VGT                                    |32   |
|2116|Alpine VGT Race                                |86   |
|2117|INFINITI CONCEPT VGT                           |49   |
|2118|LM55 VGT                                       |21   |
|2119|Italdesign VGT Street Mode                     |135  |
|2120|Italdesign VGT Off-road Mode                   |135  |
|2121|Honda Sports VGT                               |15   |
|2122|IsoRivolta Zagato VGT                          |134  |
|2123|LF-LC GT VGT                                   |50   |
|2124|GTI Supersport VGT                             |46   |
|2127|GT-R LM NISMO '15                              |28   |
|2131|V12 Vantage GT3 '12                            |4    |
|2134|Bugatti VGT                                    |113  |
|2135|HYUNDAI N 2025 VGT                             |16   |
|2136|4C '14                                         |3    |
|2138|Mustang GT '15                                 |13   |
|2139|RC F '14                                       |50   |
|2141|Golf VII GTI '14                               |46   |
|2142|NISSAN CONCEPT 2020 VGT                        |28   |
|2143|R8 LMS '15                                     |5    |
|2144|S-FR '15                                       |43   |
|2145|Focus ST '15                                   |13   |
|2146|F-type R '14                                   |17   |
|2147|Veneno '14                                     |112  |
|2148|Roadster S (ND) '15                            |21   |
|2149|Mercedes-AMG GT S '15                          |153  |
|2150|Lancer Evolution Final '15                     |25   |
|2152|Charger SRT Hellcat '15                        |11   |
|2153|WRX STI Type S '14                             |38   |
|2154|86 GT '15                                      |43   |
|2155|Polo GTI '14                                   |46   |
|2156|2&4 powered by RC213V                          |15   |
|2157|V8 Vantage Gr.4                                |4    |
|2158|458 Italia GT3 '13                             |110  |
|2159|Mustang Gr.3                                   |13   |
|2160|Genesis Gr.3                                   |16   |
|2161|GT-R Gr.4                                      |28   |
|2162|LaFerrari '13                                  |110  |
|2163|Genesis Gr.4                                   |16   |
|2164|Mustang Gr.4                                   |13   |
|2166|4C Gr.4                                        |3    |
|2167|GT-R '17                                       |28   |
|2169|TTS Coupe '14                                  |5    |
|2170|A 45 AMG '13                                   |153  |
|2171|Huracan LP 610-4 '15                           |112  |
|2172|DS 3 Racing '11                                |154  |
|2173|Atenza Sedan XD L Package '15                  |21   |
|2174|650S '14                                       |117  |
|2175|V8 Vantage S '15                               |4    |
|2176|RCZ GT Line '15                                |32   |
|2177|Huracan GT3 '15                                |112  |
|2178|S-FR Racing Concept '16                        |43   |
|2179|Bugatti VGT (Gr.1)                             |113  |
|2180|HYUNDAI N 2025 VGT (Gr.1)                      |16   |
|2181|LM55 VGT (Gr.1)                                |21   |
|2182|650S GT3 '15                                   |117  |
|2183|Corvette C7 Gr.3                               |7    |
|2184|F-type Gr.3                                    |17   |
|2185|Lancer Evolution Final Gr.3                    |25   |
|2186|WRX Gr.3                                       |38   |
|2187|FT-1 VGT (Gr.3)                                |43   |
|2188|R.S.01 GT3 '16                                 |34   |
|2190|GT by Citroen Race Car (Gr.3)                  |9    |
|2192|Volkswagen GTI VGT (Gr.3)                      |46   |
|3183|PEUGEOT VGT (Gr.3)                             |32   |
|3185|4C Gr.3                                        |3    |
|3187|Alpine VGT '17                                 |86   |
|3188|SRT Tomahawk VGT (Gr.1)                        |11   |
|3192|SLS AMG Gr.4                                   |153  |
|3209|M4 Safety Car                                  |6    |
|3210|Mercedes-AMG GT Safety Car                     |153  |
|3214|Civic Type R (FK2) '15                         |15   |
|3215|208 GTi by Peugeot Sport '14                   |32   |
|3216|R.S.01 '16                                     |34   |
|3217|Camaro SS '16                                  |7    |
|3218|M6 GT3 Endurance Model '16                     |6    |
|3219|NSX '17                                        |15   |
|3220|Clio R.S. 220 Trophy '15                       |34   |
|3221|M6 GT3 Sprint Model '16                        |6    |
|3222|86 GRMN '16                                    |43   |
|3223|Viper SRT GT3-R '15                            |11   |
|3224|Mercedes-AMG GT3 '16                           |153  |
|3225|GT-R Safety Car                                |28   |
|3227|LC500 '17                                      |50   |
|3228|RC F GT3 prototype '16                         |50   |
|3229|Mustang Gr.B Rally Car                         |13   |
|3230|Lancer Evolution Final Gr.B Rally Car          |25   |
|3231|Scirocco Gr.4                                  |46   |
|3232|WRX Gr.B Rally Car                             |38   |
|3234|Genesis Gr.B Rally Car                         |16   |
|3235|NSX Gr.3                                       |15   |
|3237|Atenza Gr.3                                    |21   |
|3238|RCZ Gr.3                                       |32   |
|3239|NSX Gr.B Rally Car                             |15   |
|3241|GT-R Gr.B Rally Car                            |28   |
|3242|RCZ Gr.B Rally Car                             |32   |
|3245|M4 Gr.4                                        |6    |
|3246|Veyron Gr.4                                    |113  |
|3247|Corvette C7 Gr.4                               |7    |
|3248|GT by Citroen Gr.4                             |9    |
|3249|Viper Gr.4                                     |11   |
|3251|NSX Gr.4                                       |15   |
|3252|F-type Gr.4                                    |17   |
|3253|Huracan Gr.4                                   |112  |
|3254|RC F Gr.4                                      |50   |
|3256|Atenza Gr.4                                    |21   |
|3257|650S Gr.4                                      |117  |
|3258|Lancer Evolution Final Gr.4                    |25   |
|3259|RCZ Gr.4                                       |32   |
|3260|Megane Gr.4                                    |34   |
|3261|WRX Gr.4                                       |38   |
|3262|86 Gr.4                                        |43   |
|3263|458 Italia Gr.4                                |110  |
|3264|Focus Gr.B Rally Car                           |13   |
|3265|86 Gr.B Rally Car                              |43   |
|3266|i3 '15                                         |6    |
|3267|F12berlinetta '12                              |110  |
|3268|911 GT3 RS (991) '16                           |136  |
|3295|86 GT 'Limited' '16                            |43   |
|3296|Corvette C7 Gr.3 Road Car                      |7    |
|3297|WRX STI Isle of Man '16                        |38   |
|3298|TT Cup '16                                     |5    |
|3299|4C Gr.3 Road Car                               |3    |
|3300|Mustang Gr.3 Road Car                          |13   |
|3301|Lancer Evolution Final Gr.B Road Car           |25   |
|3303|RCZ Gr.3 Road Car                              |32   |
|3304|WRX Gr.B Road Car                              |38   |
|3305|Beetle Gr.3                                    |46   |
|3306|Atenza Gr.3 Road Car                           |21   |
|3309|Vulcan '16                                     |4    |
|3310|Cayman GT4 Clubsport '16                       |136  |
|3311|911 RSR (991) '17                              |136  |
|3312|TS050 - Hybrid '16                             |43   |
|3313|919 Hybrid '16                                 |136  |
|3314|Audi VGT                                       |5    |
|3315|McLaren VGT                                    |117  |
|3316|COPEN RJ VGT                                   |10   |
|3332|L500R HYbrid VGT 2017                          |32   |
|3333|L750R HYbrid VGT 2017                          |32   |
|3334|R18 '16                                        |5    |
|3335|McLaren VGT (Gr.1)                             |117  |
|3336|F-150 SVT Raptor '11                           |13   |
|3337|A110 '17                                       |86   |
|3338|CHC 1967 Chevy Nova                            |147  |
|3339|BRZ Drift Car '17                              |38   |
|3340|RC F GT3 '17                                   |50   |
|3341|Pantera '71                                    |140  |
|3342|F1500T-A                                       |33   |
|3343|DB11 '16                                       |4    |
|3344|M3 Sport Evolution '89                         |6    |
|3345|GT-R NISMO '17                                 |28   |
|3346|Mach Forty                                     |148  |
|3348|NSX CONCEPT-GT '16                             |15   |
|3349|RC F GT500 '16                                 |50   |
|3350|GT-R NISMO GT500 '16                           |28   |
|3351|Audi e-tron VGT                                |5    |
|3352|GR Supra Racing Concept '18                    |43   |
|3353|Clio R.S. 220 Trophy '16                       |34   |
|3354|BRZ S '15                                      |38   |
|3356|Mini-Cooper 'S' '65                            |51   |
|3357|S660 '15                                       |15   |
|3358|911 GT3 (996) '01                              |136  |
|3359|911 GT3 (997) '09                              |136  |
|3360|McLaren P1 GTR '16                             |117  |
|3361|E-type Coupe '61                               |17   |
|3362|F50 '95                                        |110  |
|3363|DB3S '53                                       |4    |
|3364|Greddy Fugu Z                                  |149  |
|3365|356 A/1500 GS GT Carrera Speedster '56         |136  |
|3367|GR Supra RZ '19                                |43   |
|3368|Tundra TRD Pro '19                             |43   |
|3369|SR3 SL '13                                     |141  |
|3370|Fit Hybrid '14                                 |15   |
|3371|SF19 Super Formula / Toyota '19                |143  |
|3372|SF19 Super Formula / Honda '19                 |143  |
|3373|962 C '88                                      |136  |
|3374|Red Bull X2019 Competition                     |33   |
|3375|356 A/1500 GS Carrera '56                      |136  |
|3376|D-type '54                                     |17   |
|3377|Super Bee '70                                  |11   |
|3383|Demio XD Touring '15                           |21   |
|3384|GTO Twin Turbo '91                             |25   |
|3385|911 Turbo (930) '81                            |136  |
|3387|Camaro ZL1 1LE Package '18                     |7    |
|3388|300 SEL 6.8 AMG '71                            |153  |
|3389|M3 '03                                         |6    |
|3390|Taycan Turbo S '19                             |136  |
|3391|Shelby GT350R '16                              |13   |
|3392|Aventador LP 750-4 SV '15                      |112  |
|3393|Carrera GT '04                                 |136  |
|3394|DBR9 GT1 '10                                   |4    |
|3396|Jaguar VGT Coupe                               |17   |
|3397|CLK-LM '98                                     |22   |
|3398|CTR3 '07                                       |35   |
|3399|GR Supra Race Car '19                          |43   |
|3400|911 GT1 Strassenversion '97                    |136  |
|3401|Crown Athlete G '13                            |43   |
|3402|Ford GT '17                                    |13   |
|3403|Golf I GTI '83                                 |46   |
|3404|911 Carrera RS CS (993) '95                    |136  |
|3405|R8 LMS Evo '19                                 |5    |
|3406|Charger SRT Hellcat Safety Car                 |11   |
|3407|Megane R.S. Trophy Safety Car                  |34   |
|3408|Crown Athlete G Safety Car                     |43   |
|3409|Mono '16                                       |144  |
|3410|917K '70                                       |136  |
|3411|Giulia GTAm '20                                |3    |
|3412|R8 Coupé V10 plus '16                          |5    |
|3413|BRZ STI Sport '18                              |38   |
|3414|Lambo V12 VGT                                  |112  |
|3415|8C 2900B Touring Berlinetta '38                |3    |
|3416|Mercedes-AMG GT R '17                          |153  |
|3417|Jaguar VGT SV                                  |17   |
|3418|GR Supra RZ '20                                |43   |
|3419|RX-VISION GT3 CONCEPT                          |21   |
|3420|Focus RS '18                                   |13   |
|3421|Merak SS '80                                   |116  |
|3422|3.0 CSL '73                                    |6    |
|3423|Wicked Fabrication GT 51                       |150  |
|3424|Lancer Evolution IX MR GSR '06                 |25   |
|3426|Roadster Shop Rampage                          |151  |
|3427|RX-8 Spirit R '12                              |21   |
|3428|S Barker Tourer '29                            |22   |
|3429|Fairlady Z 432 '69                             |28   |
|3430|Willys MB '45                                  |146  |
|3431|911 Carrera RS (964) '92                       |136  |
|3432|Impreza Sedan WRX STi '04                      |38   |
|3433|FXX K '14                                      |110  |
|3434|Testarossa '91                                 |110  |
|3436|Ford GT Race Car '18                           |13   |
|3437|Mangusta '69                                   |140  |
|3438|Abarth 595 SS '70                              |125  |
|3439|911 Carrera RS (993) '95                       |136  |
|3441|300 SL (W194) '52                              |22   |
|3442|A112 Abarth '85                                |57   |
|3443|308 GTB '75                                    |110  |
|3444|1932 Ford Roadster Hot Rod                     |13   |
|3445|DB5 '64                                        |4    |
|3446|Spyder type 550/1500RS '55                     |136  |
|3449|Corvette C7 ZR1 '19                            |7    |
|3450|GT-R NISMO GT3 '18                             |28   |
|3451|GR Yaris RZ 'High performance' '20             |43   |
|3452|917 LIVING LEGEND                              |136  |
|3453|M3 '89                                         |6    |
|3454|3.0 CSL '71                                    |6    |
|3456|Swift Sport '17                                |39   |
|3457|A220 Race Car '68                              |86   |
|3458|Mercedes-AMG C 63 S '15                        |153  |
|3459|918 Spyder '13                                 |136  |
|3462|GTO 'The Judge' '69                            |52   |
|3464|Corvette (C1) '58                              |7    |
|3466|Sierra RS 500 Cosworth '87                     |13   |
|3467|Civic Type R Limited Edition (FK8) '20         |15   |
|3468|RGT 4.2 '16                                    |35   |
|3469|F8 Tributo '19                                 |110  |
|3471|Celica GT-Four (ST205) '94                     |43   |
|3473|Chiron '16                                     |113  |
|3474|BRZ GT300 '21                                  |38   |
|3475|MP4/4 '88                                      |117  |
|3476|Suzuki Vision Gran Turismo                     |39   |
|3477|Silvia spec-R Aero (S15) Touring Car           |28   |
|3478|Porsche VGT                                    |136  |
|3479|Jaguar VGT Roadster                            |17   |
|3480|Swift Sport Gr.4                               |39   |
|3481|GR86 RZ '21                                    |43   |
|3483|M2 Competition '18                             |6    |
|3485|Mercedes-AMG GT Black Series '20               |153  |
|3486|Challenger SRT Demon '18                       |11   |
|3487|Mustang Boss 429 '69                           |13   |
|3488|Cayman GT4 '16                                 |136  |
|3489|A6GCS/53 Spyder '54                            |116  |
|3490|Carrera GTS (904) '64                          |136  |
|3493|Skyline Super Silhouette Group 5 '84           |28   |
|3494|Alphard Executive Lounge '18                   |43   |
|3495|RX-VISION '15                                  |21   |
|3499|GR010 HYBRID '21                               |43   |
|3500|G70 3.3T AWD Prestige Package '22              |152  |
|3501|G70 GR4                                        |152  |
|3502|Genesis X GR3                                  |152  |
|3503|RX-VISION GT3 CONCEPT Stealth Model            |21   |
|3504|Z Performance '23                              |28   |
|3505|Mangusta (Christian Dior)                      |140  |
|3506|BRZ S '21                                      |38   |
|3507|Porsche VGT Spyder                             |136  |
|3508|SUZUKI Vision Gran Turismo (Gr.3 Version)      |39   |
|3509|911 Carrera RS (901) '73                       |136  |
|3510|Ferrari Vision Gran Turismo                    |110  |
|3511|ID.R '19                                       |46   |
|3512|Roadster NR-A (ND) '22                         |21   |
|3513|Silvia K's Type S (S14) '94                    |28   |
|3514|DS 21 Pallas '70                               |9    |
|3515|GR010 HYBRID (Olympic Esports Series) '21      |43   |
|3517|Red Bull X2019 25th Anniversary                |33   |
|3518|Corvette C8 Stingray '20                       |7    |
|3519|959 '87                                        |136  |
|3520|400R '95                                       |155  |
|3521|Mazda3 '19                                     |21   |
|3522|Maverick                                       |156  |
|3523|RS 5 Turbo DTM '19                             |5    |
|3524|GT-R NISMO (R32) '90                           |28   |
|3525|RA272 '65                                      |15   |
|3526|Civic                                          |157  |
|3528|SF23 Super Formula / Honda '23                 |143  |
|3529|SF23 Super Formula / Toyota '23                |143  |
|3530|Giulia Sprint GT Veloce '67                    |3    |
|3531|GT3 '20                                        |153  |
|3532|Valkyrie '21                                   |4    |
|3533|MC20 '20                                       |116  |
|3534|Ambulance Himedic '21                          |43   |
|3535|GR Corolla MORIZO Edition '22                  |43   |
|3536|Civic Type R (FL5) '22                         |15   |
|3537|MAZDA3 Gr.4                                    |21   |
|3538|Charger R/T 426 Hemi '68                       |11   |
|3539|911 GT3 RS (992) '22                           |136  |
|3540|Model 3 Performance '23                        |119  |
|3541|BVLGARI Aluminium VGT                          |158  |
|3542|SKODA Vision Gran Turismo                      |159  |
|3543|Genesis X Gran Berlinetta VGT Concept          |152  |
|3544|Genesis X Gran Racer Vision Gran Turismo Concept|152 |
|3545|Jimny XC '18                                   |39   |
|3546|AFEELA Prototype 2024                          |160  |
|3547|R4 GTL '85                                     |34   |
|3548|Urus '18                                       |112  |
|3550|Impreza Rally Car '98                          |38   |
|3551|M3 '97                                         |6    |
|3553|GT-R Premium edition T-spec '24                |28   |
|3554|Hiace Van DX '16                               |43   |

## Credits

Credit to [everard](https://github.com/everard/Salsa20) for the C++11 implementation of the stream cipher Salsa20.

Credit to [Nenkai](https://github.com/Nenkai/PDTools) for help with IV setup and their work on GT7 UDP Capabilities.

Credit to [ddm999](https://github.com/ddm999/gt7info) for CSV file of car and manufacturer IDs (I converted it into a markdown table).

Thanks to members of GTPlanet for their findings on this hidden gem in GT7, find the thread [here](https://www.gtplanet.net/forum/threads/gt7-is-compatible-with-motion-rig.410728/).

