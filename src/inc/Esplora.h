#ifndef ESPLORA_H_ 
#define ESPLORA_H_ 

#include "Arduino.h"
/*
 * The following constants can be used with the readButton()
 * method.
 */

const byte JOYSTICK_BASE  = 16;

const byte CH_SWITCH_1    = 0;
const byte CH_SWITCH_2    = 1;
const byte CH_SWITCH_3    = 2;
const byte CH_SWITCH_4    = 3;
const byte CH_SLIDER      = 4;
const byte CH_LIGHT       = 5;
const byte CH_TEMPERATURE = 6;
const byte CH_MIC         = 7;
const byte CH_TINKERKIT_A = 8;
const byte CH_TINKERKIT_B = 9;
const byte CH_JOYSTICK_SW = 10;
const byte CH_JOYSTICK_X  = 11;
const byte CH_JOYSTICK_Y  = 12;


const byte SWITCH_1       = 1;
const byte SWITCH_2       = 2;
const byte SWITCH_3       = 3;
const byte SWITCH_4       = 4;

const byte SWITCH_DOWN    = SWITCH_1;
const byte SWITCH_LEFT    = SWITCH_2;
const byte SWITCH_UP      = SWITCH_3;
const byte SWITCH_RIGHT   = SWITCH_4;
  
const boolean PRESSED     = LOW;
const boolean RELEASED    = HIGH;

const byte JOYSTICK_DOWN  = JOYSTICK_BASE;
const byte JOYSTICK_LEFT  = JOYSTICK_BASE+1;
const byte JOYSTICK_UP    = JOYSTICK_BASE+2;
const byte JOYSTICK_RIGHT = JOYSTICK_BASE+3;

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

class _Esplora {
private:
  byte lastRed;
  byte lastGreen;
  byte lastBlue;

public:
  _Esplora();
  int readSlider();
  int readLightSensor();
  int readTemperature(byte scale);
  int readMicrophone();
  int readJoystickSwitch();
  int readJoystickButton();
  int readAccelerometer(byte axis);
  bool joyLowHalf(byte joyCh);
  bool joyHighHalf(byte joyCh);
  bool readButton(byte button);
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
  unsigned int readTinkerkitInput(byte whichInput);
  unsigned int readTinkerkitInputA();
  unsigned int readTinkerkitInputB();
};

extern _Esplora Esplora;
#endif