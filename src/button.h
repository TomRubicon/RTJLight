#pragma once
#include <OneButton.h>

OneButton btn = OneButton {
  BUTTON_PIN,  // Input pin
  false,       // Button is active HIGH
  false        // Disable internal pull-up resistor
};

// Single Click rotates through preprogrammed animations
static void handleClick() {
  Serial.println("Button Single Click");
  g_State += 1;
  if (g_State > g_StateMax) g_State = 0;
}


// Double Clicking the button engages Network Mode
static void handleDoubleClick()  {
  Serial.println("Double click");
  g_bNetworkMode = !g_bNetworkMode;
}

// Long pressing the button engages Access Point Mode
static void handleLongPress() {
  Serial.println("Button Long Press");
  WiFi.disconnect();
  initAP();
}