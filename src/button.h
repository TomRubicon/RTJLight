#pragma once
#include <OneButton.h>
#include <Arduino.h>

class Button {
    private:
      // Click Events
      static void handleClick(void *ptr);
      static void handleDoubleClick(void *ptr);
      static void handleLongPress(void *ptr);

      OneButton _btn;

      // Reference variables
      int &_state;          
      int &_stateMax;
      bool &_bNetworkMode;

    public:
      Button(int buttonPin, int &state, int &stateMax, bool &bNetworkMode):
        _state(state),
        _stateMax(stateMax),
        _bNetworkMode(bNetworkMode) 
      {
        _btn = OneButton {
          buttonPin, // Input Pin   
          false,     // Button is active HIGH
          false      // Disable internal pull-up resistor
        };

        // Attach click events
        _btn.attachClick(handleClick, this);
        _btn.attachDoubleClick(handleDoubleClick, this);
        _btn.attachLongPressStart(handleLongPress, this);
      }

      ~Button();

      void tick();
};

// Single Click rotates through preprogrammed animations
void Button::handleClick(void *ptr) {
  Serial.println("Button Single Click");
  Button *buttonPtr = (Button *)ptr;
  buttonPtr->_state += 1;
  if (buttonPtr->_state > buttonPtr->_stateMax) buttonPtr->_state = 0;
}

// Double Clicking the button engages Network Mode
void Button::handleDoubleClick(void *ptr) {
  Serial.println("Button Double Click");
  Button *buttonPtr = (Button *)ptr;
  buttonPtr->_bNetworkMode = !buttonPtr->_bNetworkMode;
}

// Long pressing the button engages Access Point Mode
void Button::handleLongPress(void *ptr) {
  Serial.println("Button Long press");
  Button *buttonPtr = (Button *)ptr;
  // Reconnect these next two lines once they are also in a seperate file
  //WiFi.disconnect();
  //initAP();
}

void Button::tick() {
  _btn.tick();
}

Button::~Button() {

}
