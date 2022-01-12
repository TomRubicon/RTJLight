#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <OneButton.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>

#define LED_BUILTIN                2
#define BUTTON_PIN                12
#define LEDS_PIN                   5
#define NUM_LEDS                  98
#define HTTP_PORT                 80 
#define WS_PORT                 5678

static byte g_State         =      2;
static byte g_StateMax      =      2;
CRGB g_LEDs[NUM_LEDS]       =     {0};
int g_Brightness            =    120;
int g_BrightnessMax         =    120;
int g_PowerLimit            =   1200;
int g_paletteIndex          =      0;

bool apMode = false;
bool networkMode = false;

IPAddress local_IP(192, 168, 0, 184);
IPAddress  gateway(192, 168, 0,   1);
IPAddress   subnet(255, 255, 0,   0);

AsyncWebServer server(HTTP_PORT);

WebSocketsClient webSocket;
unsigned long    messageInterval =    30;
bool             wsConnected     = false;

#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000/x)

#include "palettes.h"

// SPIFFS & EEPROM  ////////////////////////////////////////////////////////////
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

// WEB SERVER  /////////////////////////////////////////////////////////////////
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

// WEB SOCKET CLIENT  //////////////////////////////////////////////////////////
void hexDump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08x len: 0x%X (%d)", (ptrdiff_t)src, len, len);

  for(uint32_t i = 0; i <  len; i++) {
    if(i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02x ", *src);
    src++;
  }
  Serial.print("\n");
}

void handleJSON(uint8_t * payload) {
      char* input;
      size_t inputLength;
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, input, inputLength);
      deserializeJson(doc, payload);
      Serial.println("Deserialized Json");
      for (JsonPair item : doc.as<JsonObject>()) {
        const char* item_key = item.key().c_str();
        int value_r = item.value()["r"];
        int value_g = item.value()["g"];
        int value_b = item.value()["b"];

        Serial.println(item_key);
        Serial.println(value_r);
        Serial.println(value_g);
        Serial.println(value_b);

        int id = atoi(item_key);
        Serial.println(id);
        g_LEDs[id] = CRGB(value_r, value_g, value_b);
      }
      doc.clear();
      FastLED.show();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type){
    case WStype_DISCONNECTED:
      Serial.println("[WEBSOCKET] Disconnected!");
      wsConnected = false;
      break;

    case WStype_CONNECTED:
      Serial.printf("[WEBSOCKET] Connected to URL: %s\n", payload);
      wsConnected = true;

      Serial.println("[WEBSOCKET] SENT: Connected");
      webSocket.sendTXT("Connected");
      break;
      
    case WStype_TEXT:
      Serial.printf("[WEBSOCKET] Response: %s\n", payload); 
      handleJSON(payload);
      break;

    case WStype_BIN:
      Serial.printf("[WEBSOCKET] get binary length: %u\n", length);
      hexDump(payload, length);
      break;

    case WStype_PING:
      Serial.println("[WEBSOCKET] Ping!");
      break;

    case WStype_PONG:
      Serial.println("[WEBSOCKET] Pong!");
      break;

    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
  }
}

void initWebSocket() {
  webSocket.begin("192.168.0.51", WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  Serial.println("[WEBSOCKET] Websocket trying to connect");
}

// WIFI  ///////////////////////////////////////////////////////////////////////
bool initAP() {
  Serial.println("No SSID or Password in memory. Loading form...");

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
  apMode = true;

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

// BUTTON  /////////////////////////////////////////////////////////////////////
OneButton btn = OneButton {
  BUTTON_PIN,
  false,
  true
};

static void handleClick() {
  Serial.println("Button Single Click");
  g_State += 1;
  if (g_State > g_StateMax) g_State = 0;
}

static void handleLongPress() {
  Serial.println("Button Long Press");
  WiFi.disconnect();
  initAP();
}

// MAIN  ///////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(LEDS_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("ESP32 Starting up");

  initSPIFFS();
  EEPROM.begin(400);

  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setBrightness(g_Brightness);
  set_max_power_indicator_LED(LED_BUILTIN);
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit);
  g_LEDs[0] = CRGB::Yellow;
  FastLED.show();

  btn.attachClick(handleClick);
  btn.attachLongPressStart(handleLongPress);

  initWifi();
}

void loop() {
  while (true) {
    btn.tick();

    g_Brightness += 1;
    if(g_Brightness > g_BrightnessMax) g_Brightness = g_BrightnessMax;

    if(apMode) {  // AP MODE ///////////////////////////////////////////////////
      uint8_t sinBeat = beatsin8(15, 0, NUM_LEDS - 1, 0, 0);
      uint8_t sinBeat2 = beatsin8(15, 0, NUM_LEDS -1, 0, 127);

      g_LEDs[sinBeat] = CRGB::Purple;
      g_LEDs[sinBeat2] = CRGB::Gold;

      fadeToBlackBy(g_LEDs, NUM_LEDS, 2);

      FastLED.show();

    } else if(networkMode) {  //  Network Mode  ////////////////////////////////
      webSocket.loop();

      FastLED.show();

    } else {  // Standard Mode  ////////////////////////////////////////////////
      
      if(g_State == 0) {
        fill_palette(g_LEDs, NUM_LEDS, g_paletteIndex, 255 / NUM_LEDS, yankeeBrave, 100, LINEARBLEND);
        FastLED.delay(5);
      }
      else if(g_State == 1) {
        fill_palette(g_LEDs, NUM_LEDS, g_paletteIndex, 255 / NUM_LEDS, greenPinkPurple, 100, LINEARBLEND);
        FastLED.delay(10);
      }
      else if(g_State == 2)  {
        fill_rainbow(g_LEDs, NUM_LEDS, g_paletteIndex, 255 / NUM_LEDS);
        FastLED.delay(20);
      }

    }
    g_paletteIndex++;

    FastLED.setBrightness(g_Brightness);

  }
}