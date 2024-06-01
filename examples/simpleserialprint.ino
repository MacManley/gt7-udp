//File: simpleserialprint.ino

//#include "Wifi.h" // ESP32 WiFi include
#include <ESP8266WiFi.h> // ESP8266 WiFi include
#include <GT7UDPParser.h>

const char *SSID = "Your WiFi SSID";
const char *Password = "Your WiFi Password";
const IPAddress ip(..., ..., ., ..); // Insert your PS4/5 IP address here

void startWiFi();

unsigned long previousT = 0; 
const long interval = 500; 

GT7_UDP_Parser gt7Telem;
Packet packetContent;

void setup()
{
  Serial.begin(115200);
  startWiFi();
  gt7Telem.begin(ip);
  gt7Telem.sendHeartbeat();
}

void loop()
{
  unsigned long currentT = millis();
  packetContent= gt7Telem.readData();

    float speed = (packetContent.packetContent.speed) * 3.6; // Times by 3.6 to convert from m/s to km/h

    Serial.print("Speed: ");
    Serial.println(speed);

  if (currentT - previousT >= interval)
  { // Send heartbeat every 500ms
    previousT = currentT;
    gt7Telem.sendHeartbeat();
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
