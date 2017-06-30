#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "Arduino.h"
#include "Device.h"
#include "Serial.h"
#include "WString.h"
#include "ultoa.h"

void _Serial::begin(unsigned long baud_rate) {
  for (int i = 0; i < 12; i++) {
    if (baud_rate == _possible_bauds[i]) {
      _baud_rate = baud_rate;
      return;
    }
  }
  _baud_rate = 9600;
}


void _Serial::end() {
}

// returns the number of bytes available to read
// no bytes available, doesn't wait
int _Serial::available() {
  _fill();
  return _in != -1;
}

// the first byte of incoming serial data available
// Data is never available
int _Serial::read() {
  _fill();
  int x = _in;
  _in = -1;
  return x;
}

int _Serial::peek() {
  _fill();
  return _in;
}

void _Serial::write(uint8_t c) {
  putchar(c);
}

void _Serial::flush() {
}

void _Serial::_fill() {
  if (_in >= 0) {
    return;
  }
  _in = getchar();
  if (_in == EOF) {
    _in = -1;
  }
}
