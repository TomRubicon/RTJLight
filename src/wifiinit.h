#pragma once
#include<WiFi.h>

IPAddress local_IP(192, 168, 0, 184);
IPAddress  gateway(192, 168, 0,   1);
IPAddress   subnet(255, 255, 0,   0);

bool initAP() {
  Serial.println("No SSID or Password in memory. Loading form...");

  // Set LEDs to purple
  FastLED.clear();
  g_LEDs[0] = CRGB::Purple;
  FastLED.show();
  
  const char* ssid     = "RTJ Config";
  const char* password = "OohLaLaAhOuiOui";

  Serial.println("Starting up access point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();

  Serial.print("AP IP Address: ");
  Serial.print(IP);
  Serial.print("\n");

  initWebServer();
  g_apMode = true;

  return false;
}

void initWifi() {
  Serial.println("Checking SSID and Password...");
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);

  Serial.print("Found SSID and Password: ");
  Serial.print(s);
  Serial.print('/');
  Serial.print(p);
  Serial.print(" \n");
  delay(1000);

  if(s.length() > 0 && p.length() > 0){

  }
  else {
    initAP();
    return;
  }

  char ssid[s.length()];
  char password[p.length()];

  s.toCharArray(ssid, s.length() + 1);
  p.toCharArray(password, p.length() + 1);

  Serial.println("Converted SSID and password:");
  Serial.println(ssid);
  Serial.println(password);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  int i = 1;

  while(WiFi.status() != WL_CONNECTED) {
    for(int x = 0; x <= i; x++)
    {
      g_LEDs[x] = CRGB::Green;
    }
    i++;
    if(i > 98) i = 1;

    FastLED.show();
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  FastLED.clear();
  FastLED.show();
  g_Brightness = 0;
  delay(1000);
  
  Serial.println(WiFi.localIP());

  initWebSocket();
}
