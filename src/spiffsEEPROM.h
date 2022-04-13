#pragma once
#include <SPIFFS.h>
#include <EEPROM.h>

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Cannot mount SPIFFS volume...");
  }
}

void wipeEEPROM() {
  for(int i=0; i<400; i++) {
    EEPROM.writeByte(i, 0);
  }
  EEPROM.commit();
}

bool writeToMemory(String ssid, String pass) {
  char sBuff[30];
  char pBuff[30];
  ssid.toCharArray(sBuff, 30);
  pass.toCharArray(pBuff, 30);
  EEPROM.writeString(100, sBuff);
  EEPROM.writeString(200, pBuff);
  delay(100);
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);

  Serial.print("Stored SSID and Password: ");
  Serial.print(s);
  Serial.print(" / ");
  Serial.print(p);
  Serial.println("");

  if(ssid == s && pass == p) {
    return true;
  } else {
    return false;
  }
}
