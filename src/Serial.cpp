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
  return 1;
}

// the first byte of incoming serial data available 
// Data is never available
int _Serial::read() {
  return (-1);
}

int _Serial::peek() {
  return (-1);
}

void _Serial::write(uint8_t c) {
  putchar(c);
}

void _Serial::flush() {
}
