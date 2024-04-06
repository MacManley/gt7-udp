//File: simulatorflags.ino

//#include "Wifi.h" // ESP32 WiFi include
#include <ESP8266WiFi.h> // ESP8266 WiFi include
#include <WiFiUdp.h>
#include <GT7UDPParser.h>

const char *SSID = "Your WiFi SSID";
const char *Password = "Your WiFi Password";
const IPAddress psGTServerIP(..., ..., ., ..); // Insert your PS4/5 IP address here
char replyPacket[] = "A";

void startWiFi();

WiFiUDP Udp;

unsigned int localPort = 33740; // Port that is used in game, 33740
unsigned int remotePort = 33739; // Port that is to receive the heartbeat, 33739
unsigned long previousT = 0;
const long interval = 500;

GT7_UDP_Parser* parser;

void setup()
{
  parser = new GT7_UDP_Parser();
  Serial.begin(115200);
  startWiFi();
  Udp.begin(localPort);
  delay(500);
  Udp.beginPacket(psGTServerIP, remotePort);
  Udp.write(replyPacket);
  Udp.endPacket();
}

void loop()
{
  unsigned long currentT = millis();
  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    uint8_t packetBuffer[packetSize];
    while (Udp.available())
    {
      Udp.read(packetBuffer, packetSize);
    }

    parser->push(packetBuffer);

    for (int i = 0; i < 13; ++i)
    {
      uint8_t flag = parser->packetInfo()->getFlag(i);
      Serial.print("Flag ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(flag);
    }
    
    // This is how you would implement flags in your Arduino code without using the function:

    /*SimulatorFlags flags = parser->packetInfo()->m_packet.flags;

    if (static_cast<int16_t>(flags) & static_cast<int16_t>(SimulatorFlags::TCSActive)) {
      Serial.println("FLAG: ON");
    } else {
      Serial.println("FLAG: OFF");
    }*/

    if (currentT - previousT >= interval)
    {
      previousT = currentT;
      Udp.beginPacket(psGTServerIP, remotePort);
      Udp.write(replyPacket);
      Udp.endPacket();
      Serial.println("Pckt sent");
    }
  }
}

void startWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, Password);
  Serial.print("Attempting to connect to ");
  Serial.println(SSID);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(250);

    if ((++i % 16) == 0)
    {
      Serial.println(F(" still trying to connect"));
    }
  }

  Serial.print(F("Connection Successful | IP Address: "));
  Serial.println(WiFi.localIP());
}
