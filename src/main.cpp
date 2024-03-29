#include <stdlib.h>
#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

#define LED_BUILTIN                2
#define BUTTON_PIN                12
#define LEDS_PIN                   5
#define NUM_LEDS                  98
#define HTTP_PORT                 80 
#define WS_PORT                 5678

int g_State          =      2;
int g_StateMax       =      2;
CRGB g_LEDs[NUM_LEDS]       =     {0};
int g_Brightness            =    120;
int g_BrightnessMax         =    120;
int g_PowerLimit            =   1200;
int g_paletteIndex          =      0;

bool g_apMode = false;
bool g_bNetworkMode = false;

#include "palettes.h"
#include "spiffsEEPROM.h"
#include "webserver.h"
#include "websocket.h"
#include "wifiinit.h"
#include "button.h"

void setup() {
  pinMode(LEDS_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("ESP32 Starting up");

  // Setup SPIFFS and EEPROM
  initSPIFFS();
  EEPROM.begin(400);

  // Setup Button
  btn.attachClick(handleClick);
  btn.attachDoubleClick(handleDoubleClick);
  btn.attachLongPressStart(handleLongPress);
  
  // Setup LED strip
  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setBrightness(g_Brightness);
  set_max_power_indicator_LED(LED_BUILTIN);
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit);
  g_LEDs[0] = CRGB::Yellow;
  FastLED.show();

  initWifi();
}

void loop() {
  while (true) {
    btn.tick();

    g_Brightness += 1;
    if(g_Brightness > g_BrightnessMax) g_Brightness = g_BrightnessMax;

    if(g_apMode) {  // AP MODE ///////////////////////////////////////////////////
      uint8_t sinBeat = beatsin8(15, 0, NUM_LEDS - 1, 0, 0);
      uint8_t sinBeat2 = beatsin8(15, 0, NUM_LEDS -1, 0, 127);

      g_LEDs[sinBeat] = CRGB::Purple;
      g_LEDs[sinBeat2] = CRGB::Gold;

      fadeToBlackBy(g_LEDs, NUM_LEDS, 2);

      FastLED.show();

    } else if(g_bNetworkMode) {  //  Network(Websocket) Mode  ////////////////////////////////
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