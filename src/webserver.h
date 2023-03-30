#pragma once
#include <ESPAsyncWebServer.h>

AsyncWebServer server(HTTP_PORT);

void initWebServer(bool forAP = true) {
  if(forAP) {
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request){
      Serial.println("GET REQUEST");
      if (request->hasParam("ssid") && request->hasParam("password")) {
        Serial.println("REQUEST INCLUDES SSID AND PASSWORD!");
        Serial.println(String(request->getParam("ssid")->value()));
        Serial.println(String(request->getParam("password")->value()));

        String response_success = "<h1>Success</h1>";
        response_success += "<h2><b>Device will reset in 3 seconds</b></h2>";

        String response_error   = "<h1>ERROR</h1>";
        response_error += "<h2><a href='/'>Go back</a></h2>";

        if(writeToMemory(String(request->getParam("ssid")->value()), String(request->getParam("password")->value()))) {
          request->send(200, "text/html", response_success);

          FastLED.clear();
          g_LEDs[0] = CRGB::Blue;
          FastLED.show();

          EEPROM.commit();
    
          delay(3000);

          FastLED.clear();

          ESP.restart();
        } else {
          request->send(200, "text/html", response_error);
        }
      }
    });
  }

  server.begin();
  Serial.println("HTTP server started!");
}
