//File: simulatorflags.ino

//#include "Wifi.h" // ESP32 WiFi include
#include <ESP8266WiFi.h> // ESP8266 WiFi include
#include <GT7UDP.h>

const char *SSID = "Your WiFi SSID";
const char *Password = "Your WiFi Password";
const IPAddress ip(..., ..., ., ..); // Enter your Playstations IP here

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
  packetContent = gt7Telem.readData(); 

    for (int i = 0; i < 13; ++i)
    {
      uint8_t flag = gt7Telem.getFlag(i);
      Serial.print("Flag ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(flag);
    }

    if (currentT - previousT >= interval)
    {
      previousT = currentT;
      gt7Telem.sendHeartbeat();
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
