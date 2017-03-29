#ifndef ESPLORA_H_ 
#define ESPLORA_H_ 

#include "esplora_pins.h"
#include "Arduino.h"
/*
 * The following constants can be used with the readButton()
 * method.
 */

const byte SWITCH_1       = 1;
const byte SWITCH_2       = 2;
const byte SWITCH_3       = 3;
const byte SWITCH_4       = 4;

const byte SWITCH_DOWN  = SWITCH_1;
const byte SWITCH_LEFT  = SWITCH_2;
const byte SWITCH_UP    = SWITCH_3;
const byte SWITCH_RIGHT = SWITCH_4;

const boolean PRESSED   = LOW;
const boolean RELEASED  = HIGH;

/*
 * The following constants can be used with the readTemperature()
 * method to specify the desired scale.
 */
const byte DEGREES_C = 0;
const byte DEGREES_F = 1;

/*
 * The following constants can be used with the readAccelerometer()
 * method to specify the desired axis to return.
 */
const byte X_AXIS = 0;
const byte Y_AXIS = 1;
const byte Z_AXIS = 2;

typedef uint8_t byte;

class esplora {
private:
  byte lastRed;
  byte lastGreen;
  byte lastBlue;
  uint32_t _duration_left;

public:
  int readSlider();
  int readLightSensor();
  int readTemperature(byte scale);
  int readMicrophone();
  int readJoystickSwitch();
  int readJoystickButton();
  int readAccelerometer(byte axis);
  int readButton(byte button);
  int readJoystickX();
  int readJoystickY();
  void writeRGB(byte red, byte green, byte blue);
  void writeRed(byte red);
  void writeGreen(byte green);
  void writeBlue(byte blue);
  byte readRed();
  byte readGreen();
  byte readBlue();
  void noTone();
  void tone(unsigned int freq);
  void tone(unsigned int freq, unsigned long duration);
};

#endif