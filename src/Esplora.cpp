/*
  Esplora.cpp - Arduino simulator Esplora board library
  Written by Owen Brasier

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Esplora.h"
#include "Device.h"
#include "global_variables.h"

#include <iostream>

/*
 * The following constants tell, for each accelerometer
 * axis, which values are returned when the axis measures
 * zero acceleration.
 */

const byte RED_PIN    = 5;
const byte BLUE_PIN   = 9;
const byte GREEN_PIN  = 10;

const byte BUZZER_PIN = 6;

// non-multiplexer Esplora pins:
// Accelerometer: x-A5, y-A11, z-A6
// External outputs: D3, D11
// Buzzer: D6
// RGB Led: red-D5, green-D10, blue-D9
// Led 13: D13

const byte ACCEL_X_PIN = A5;
const byte ACCEL_Y_PIN = A11;
const byte ACCEL_Z_PIN = A6;


const int JOYSTICK_DEAD_ZONE = 100;
//==========================================
// Esplora
//==========================================

_Esplora::_Esplora() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

int _Esplora::readSlider() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_SLIDER);
}

int _Esplora::readLightSensor() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_LIGHT);
}

int _Esplora::readTemperature(byte scale) {
  _sim::increment_counter(1);
  uint32_t temp = _sim::_device.get_mux_value(CH_TEMPERATURE);
  if (scale == DEGREES_F) {
    return (int)((temp * 450) / 512) - 58;
  } else {
    return (int)((temp * 500) / 1024) - 50;
  }
}

int _Esplora::readMicrophone() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_MIC);
}

int _Esplora::readJoystickSwitch() {
  _sim::increment_counter(5);
  return _sim::_device.get_mux_value(CH_JOYSTICK_SW);
}

int _Esplora::readJoystickButton() {
  _sim::increment_counter(7);
  return (_sim::_device.get_mux_value(CH_JOYSTICK_SW) == 1023) ? HIGH : LOW;
}

int _Esplora::readAccelerometer(byte axis) {
  _sim::increment_counter(1);
  switch (axis) {
    case X_AXIS: return analogRead(ACCEL_X_PIN) - 512;
    case Y_AXIS: return analogRead(ACCEL_Y_PIN) - 512;
    case Z_AXIS: return analogRead(ACCEL_Z_PIN) - 512;
    default: return 0;
  }
}

bool _Esplora::joyLowHalf(byte joyCh) {
  _sim::increment_counter(1);
  return (_sim::_device.get_mux_value(joyCh) < 512 - JOYSTICK_DEAD_ZONE)
         ? LOW : HIGH;
}

bool _Esplora::joyHighHalf(byte joyCh) {
  _sim::increment_counter(1);
  return (_sim::_device.get_mux_value(joyCh) > 512 + JOYSTICK_DEAD_ZONE)
         ? LOW : HIGH;
}

bool _Esplora::readButton(byte button) {
  if (button >= SWITCH_1 && button <= SWITCH_4) {
    button--;
  } else {

    _sim::increment_counter(1);
    return HIGH;
  }
  switch (button) {
    case JOYSTICK_RIGHT:
      return joyLowHalf(CH_JOYSTICK_X);
    case JOYSTICK_LEFT:
      return joyHighHalf(CH_JOYSTICK_X);
    case JOYSTICK_UP:
      return joyLowHalf(CH_JOYSTICK_Y);
    case JOYSTICK_DOWN:
      return joyHighHalf(CH_JOYSTICK_Y);
  }
  _sim::increment_counter(1);
  return (_sim::_device.get_mux_value(button) > 512) ? HIGH : LOW;
}

int _Esplora::readJoystickX() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_JOYSTICK_X) - 512;
}

int _Esplora::readJoystickY() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_JOYSTICK_Y) - 512;
}

void _Esplora::writeRGB(byte red, byte green, byte blue) {
  writeRed(red);
  writeGreen(green);
  writeBlue(blue);
}

// writeRed calls analogWrite to write to the correct pin
// TODO: remove led update
void _Esplora::writeRed(byte red) {
  if (red == lastRed) {
    _sim::increment_counter(1);
    return;
  }
  lastRed = red;
  analogWrite(RED_PIN, red);
  _sim::increment_counter(1);
}

void _Esplora::writeGreen(byte green) {
  if (green == lastGreen) {
    _sim::increment_counter(1);
    return;
  }
  lastGreen = green;
  analogWrite(GREEN_PIN, green);
  _sim::increment_counter(1);
}

void _Esplora::writeBlue(byte blue) {
  if (blue == lastBlue) {
    _sim::increment_counter(1);
    return;
  }
  lastBlue = blue;
  analogWrite(BLUE_PIN, blue);
  _sim::increment_counter(1);
}

byte _Esplora::readRed() {
  _sim::increment_counter(1);
  return lastRed;
}

byte _Esplora::readGreen() {
  _sim::increment_counter(1);
  return lastGreen;
}

byte _Esplora::readBlue() {
  _sim::increment_counter(1);
  return lastBlue;
}

void _Esplora::noTone() {
  ::noTone(BUZZER_PIN);
}

void _Esplora::tone(unsigned int freq) {
  if (freq > 0) {
    ::tone(BUZZER_PIN, freq);
  } else {
    ::noTone(BUZZER_PIN);
  }
}

void _Esplora::tone(unsigned int freq, unsigned long duration) {
  if (freq > 0 && duration > 0) {
    ::tone(BUZZER_PIN, freq, duration);
  } else {
    ::noTone(BUZZER_PIN);
  }
}

inline unsigned int readTinkerkitInput(byte whichInput) {
  if (whichInput < 2) {
    _sim::increment_counter(1);
    return _sim::_device.get_mux_value(whichInput + CH_TINKERKIT_A);
  }
  return 0;
}

inline unsigned int readTinkerkitInputA() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_TINKERKIT_A);
}

inline unsigned int readTinkerkitInputB() {
  _sim::increment_counter(1);
  return _sim::_device.get_mux_value(CH_TINKERKIT_B);
}
