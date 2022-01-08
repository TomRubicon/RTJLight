#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <OneButton.h>

#define LED_BUILTIN   2
#define BUTTON_PIN   12
#define LEDS_PIN      5
#define NUM_LEDS     98 

static byte g_State       =    0;
static byte g_StateMax    =    1;
CRGB g_LEDs[NUM_LEDS]     =   {0};
int g_Brightness          =  120;
int g_PowerLimit          = 1200;
int g_paletteIndex        =    0;

#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000/x)

#include "palettes.h"

OneButton btn = OneButton {
  BUTTON_PIN,
  false,
  true
};

static void handleClick() {
  g_State += 1;

  if (g_State > g_StateMax) g_State = 0;
}

void setup() {
  pinMode(LEDS_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("ESP32 Starting up");

  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setBrightness(g_Brightness);
  set_max_power_indicator_LED(LED_BUILTIN);
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit);

  btn.attachClick(handleClick);
}

void loop() {
  while (true) {
    btn.tick();

    if(g_State == 0) {
      g_paletteIndex++;
      fill_palette(g_LEDs, NUM_LEDS, g_paletteIndex, 255 / NUM_LEDS, yankeeBrave, 100, LINEARBLEND);
    }
    else if(g_State == 1) {
      g_paletteIndex++;
      fill_palette(g_LEDs, NUM_LEDS, g_paletteIndex, 255 / NUM_LEDS, greenPinkPurple, 100, LINEARBLEND);
    }

    FastLED.setBrightness(g_Brightness);
    FastLED.delay(5);

  }
}