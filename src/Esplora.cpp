/*
  Esplora.cpp - Arduino simulator Esplora board library
  Written by Owen Brasier

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
const int ACCEL_ZERO_X = 320;
const int ACCEL_ZERO_Y = 330;
const int ACCEL_ZERO_Z = 310;

const byte MUX_ADDR_PINS[] = { A0, A1, A2, A3 };
const byte MUX_COM_PIN = A4;

const int JOYSTICK_DEAD_ZONE = 100;

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

const byte LED_PIN     = 13;

//==========================================
// Esplora
//==========================================
int _Esplora::readSlider() {
  return _device.get_mux_value(CH_SLIDER);
}

int _Esplora::readLightSensor() {
  return _device.get_mux_value(CH_LIGHT);
}

int _Esplora::readTemperature(byte scale) {
  float temp = _device.get_mux_value(CH_TEMPERATURE);
  if (scale == DEGREES_F){
    return (int)(temp*9)/5 + 32;
  }
  else {
    return int(temp);
  }
}

int _Esplora::readMicrophone() {
  return _device.get_mux_value(CH_MIC);
}

int _Esplora::readJoystickSwitch() {
  return _device.get_mux_value(CH_JOYSTICK_SW);
}

int _Esplora::readJoystickButton() {
  return (_device.get_mux_value(CH_JOYSTICK_SW) == 1023) ? HIGH : LOW;
}

int _Esplora::readAccelerometer(byte axis) {
  int val;
  switch (axis) {
    case X_AXIS: val =  _device.get_pin_value(SIM_ACCEL_X); break;
    case Y_AXIS: val =  _device.get_pin_value(SIM_ACCEL_Y); break;
    case Z_AXIS: val =  _device.get_pin_value(SIM_ACCEL_Z); break;
  }
  return val;
}

int _Esplora::readButton(byte button) {
  if(button >= SWITCH_1 && button <= SWITCH_4) {
    button--;
  }
  else {
    return HIGH;
  }
  return (_device.get_mux_value(button) > 0) ? HIGH : LOW;
}

int _Esplora::readJoystickX() {
  return _device.get_mux_value(CH_JOYSTICK_X);
}

int _Esplora::readJoystickY() {
  return _device.get_mux_value(CH_JOYSTICK_Y);
}

void _Esplora::writeRGB(byte red, byte green, byte blue) {
  writeRed(red);
  writeGreen(green);
  writeBlue(blue);
}

void _Esplora::writeRed(byte red) {
  if (red == lastRed) 
    return;
  analogWrite(SIM_RED, red);
  lastRed = red;
  send_pin_update();
  int bright = map(red, 0, 255, 0, 9);
  _device.set_led(0, bright);
  send_led_update();
}

void _Esplora::writeGreen(byte green) {
  if (green == lastGreen)
    return;
  analogWrite(SIM_GREEN, green);
  lastGreen = green;
  send_pin_update();
  int bright = map(green, 0, 255, 0, 9);
  _device.set_led(1, bright);
  send_led_update();
}

void _Esplora::writeBlue(byte blue) {
  if (blue == lastBlue)
    return;
  analogWrite(SIM_BLUE, blue);
  lastBlue = blue;
  int bright = map(blue, 0, 255, 0, 9);
  _device.set_led(2, bright);
  send_led_update();
}

byte _Esplora::readRed() {
  return lastRed;
}

byte _Esplora::readGreen() {
  return lastGreen;
}

byte _Esplora::readBlue() {
  return lastBlue;
}

void _Esplora::noTone() {
  ::noTone(BUZZER_PIN);
}

void _Esplora::tone(unsigned int freq) {
  if (freq > 0) {
    ::tone(BUZZER_PIN, freq);
  }
  else {
    ::noTone(BUZZER_PIN); 
  }
}

// TODO: implement duration timeout
void _Esplora::tone(unsigned int freq, unsigned long duration) {
  if (freq > 0 && duration > 0) {
    ::tone(BUZZER_PIN, freq, duration);
  }
  else {
    ::noTone(BUZZER_PIN);
  }    
}

inline unsigned int readTinkerkitInput(byte whichInput) {
  return _device.get_mux_value(whichInput + CH_TINKERKIT_A);
}

inline unsigned int readTinkerkitInputA() {
  return _device.get_mux_value(CH_TINKERKIT_A);
}

inline unsigned int readTinkerkitInputB() {
  return _device.get_mux_value(CH_TINKERKIT_B);
}