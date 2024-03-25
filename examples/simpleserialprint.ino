//File: simpleserialprint.ino

//#include "Wifi.h" // ESP32 WiFi include
#include <ESP8266WiFi.h> // ESP8266 WiFi include
#include <WiFiUdp.h>
#include <GT7UDPParser.h>

const char *SSID = "Your WiFi SSID";
const char *Password = "Your WiFi Password";
const IPAddress psGTServerIP(..., ..., ., ..); // Insert your PS4/5 IP address here
char replyPacket[] = "A";

void startWiFi();
 
//The IP address that this ESP32 / ESP8266 has requested to be assigned to.
IPAddress ip();
WiFiUDP Udp;

unsigned int localPort = 33740; // Port that is used in game, 33739
unsigned int remotePort = 33739; // Port that is to receive the heartbeat, 33740
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
  Udp.endPacket(); // Send initial heartbeat
}

void loop()
{
  unsigned long currentT = millis();
  int packetSize = Udp.parsePacket(); 
  
    if(packetSize) 
    {
      uint8_t packetBuffer[packetSize];
      while(Udp.available())
      {
        Udp.read(packetBuffer, packetSize);
      }
    parser->push(packetBuffer);

    float speed = (parser->packetInfo()->m_packet.speed) * 3.6; // Times by 3.6 to convert from m/s to km/h

    Serial.print("Speed: ");
    Serial.println(speed);
  }

  if (currentT - previousT >= interval) { // Send heartbeat every 500ms
    previousT = currentT;
    Udp.beginPacket(psGTServerIP, remotePort);
    Udp.write(replyPacket); // Sends 'A'
    Udp.endPacket();
    //Serial.println("Pckt sent");
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