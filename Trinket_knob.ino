/*
 * PINOUT, Digispark to KY-040
 * P0   to  SW
 * P1   to  DT
 * P2   to  CLK
 * VIN  to  GND
 * GND  to  +5V
 */

//Pin settings
#define encoderPinA 2 //clk
#define encoderPinB 1 //dt
#define encoderButton 0 //sw

#define LEFT_ACTION 0x50 //Left arrow

#define RIGHT_ACTION 0x4f //Right arrow

// Debug option, uncomment next line if you want to output commands as text (e.g. to Notepad).
//#define KB_DEBUG

#ifndef KB_DEBUG
  #include "TrinketHidCombo.h"
#else
  #include <TrinketKeyboard.h>
#endif

#define LATCHSTATE 3
int buttonState = LOW, lastButtonState = LOW;
long lastDebounceTime = 0, debounceDelay = 50;
int _position = 0, _positionExt = 0;
int8_t _oldState; bool btnPressed=false, btnReleased=false, pressedRotary=false;
boolean first = true;
int mode = 1;// modes of operation 1: alt=tab 2: chrome tabs 3: volume knob

const int8_t encoderStates[] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

void setup() {
  
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(encoderButton, INPUT);
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderButton, LOW);
  _oldState = 3; 
  #ifndef KB_DEBUG
    TrinketHidCombo.begin();
  #else
    TrinketKeyboard.begin();
  #endif
}

void loop() {
  static int pos = 0;
  tick();
  int newPos = getPosition();
  if (pos != newPos) {
    #ifndef KB_DEBUG
    if (first == true && mode == 1 && !btnPressed) { //first time alt-tab and hold alt
      TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, 0x2b);
      TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, 0x0);
    }
    #endif
    if (newPos < pos) {
      if (!btnPressed) {
        #ifndef KB_DEBUG
        if (mode == 1){
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, LEFT_ACTION); // left arrow and hold alt
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, 0x0);
        } else if (mode == 2){
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_CONTROL | KEYCODE_MOD_LEFT_SHIFT, 0x2b); // press cntrl shift tab
          TrinketHidCombo.pressKey(0, 0); // release key 
        } else if (mode == 3){
          TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_DOWN); // volume down key
        }
        #else
          DigiKeyboard.println("LEFT_ACTION");
        #endif
      } else { // decrease mode count when the buton is pressed and truned 
        pressedRotary=true;
        #ifndef KB_DEBUG
          mode--;
          if (mode < 1) mode = 1;
          TrinketHidCombo.pressKey(0, 0);
        #else
          DigiKeyboard.println("LEFT_ACTION_MODE1");
          mode--;
          if (mode < 1) mode = 1;
        #endif
      }
      first = false;
    }
    else if (newPos > pos){
     if (!btnPressed) {
        #ifndef KB_DEBUG
        if (mode == 1){ // Right arrow and hold alt
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, RIGHT_ACTION);
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_ALT, 0x0);
        } else if (mode == 2){
          TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_CONTROL, 0x2b); // press cntrl tab
          TrinketHidCombo.pressKey(0, 0); // release key
        } else if (mode == 3){
          TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_UP); // volume up key
        }
        #else
          DigiKeyboard.println("RIGHT_ACTION");
        #endif
      } else { // increase mode count when the buton is pressed and truned 
        pressedRotary=true;
        #ifndef KB_DEBUG
          mode++;
          if (mode > 3) mode = 3;
          TrinketHidCombo.write(' ');
        #else
          DigiKeyboard.println("RIGHT_ACTION_MODE1");
          mode++;
          if (mode > 3) mode = 3;
        #endif
      }
      first = false;
    }
    pos = newPos;
  }
  int reading = digitalRead(encoderButton);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        btnPressed=true;
      } else if (buttonState == LOW){
        btnReleased=true;
      }
    } 
  }
  lastButtonState = reading;

  if (btnPressed == true && btnReleased ==true && pressedRotary ==false) {
    #ifndef KB_DEBUG
      if (mode == 1){ // mode 1 select screen
          TrinketHidCombo.pressKey(0, 0);
        } else if (mode == 2){ // mode 2 send spece key to play/pause
          TrinketHidCombo.pressKey(0, KEYCODE_SPACE);
          TrinketHidCombo.pressKey(0, 0);
        } else if (mode == 3){ // mode 3 send play/pause key
          TrinketHidCombo.pressMultimediaKey(MMKEY_PLAYPAUSE);
        }
      first = true;
    #else
      DigiKeyboard.println(btnPressed);
      DigiKeyboard.println(btnReleased);
      DigiKeyboard.println(pressedRotary);
      DigiKeyboard.println(mode);
      DigiKeyboard.println("BUTTON_ACTION_SINGLE");
    #endif
    btnPressed=false;btnReleased=false;
  } else if (btnPressed == true && btnReleased == true && pressedRotary == true) {
    #ifdef KB_DEBUG
      DigiKeyboard.println("RELEASED_AFTER_HELD_ROTATION");
    #endif
    btnPressed=false;btnReleased=false;pressedRotary=false;
  }
  #ifndef KB_DEBUG
    TrinketHidCombo.poll();
  #else
    TrinketKeyboard.poll();
  #endif
}

int  getPosition() {
  return _positionExt;
}

void setPosition(int newPosition) {
  _position = ((newPosition<<2) | (_position & 0x03));
  _positionExt = newPosition;
}

void tick(void) {
  int sig1 = digitalRead(encoderPinA);
  int sig2 = digitalRead(encoderPinB);
  int8_t thisState = sig1 | (sig2 << 1);
  if (_oldState != thisState) {
    _position += encoderStates[thisState | (_oldState<<2)];
    if (thisState == LATCHSTATE)
      _positionExt = _position >> 2;
    _oldState = thisState;
  }
} 
