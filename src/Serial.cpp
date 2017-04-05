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
char _Serial::read() {
  return (-1);
}

int _Serial::peek() {
  return (-1);
}

void _Serial::flush() {
}

void _Serial::print(int x) {
  digitalWrite(TX_LED, HIGH);
  std::cout << x;
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(float x) {
  digitalWrite(TX_LED, HIGH);
  std::cout << x;
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(std::string s) {
  digitalWrite(TX_LED, HIGH);
  const char *p;
  p = s.c_str();
  std::cout << p;
  fflush(stdout);
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(unsigned int x, int base) {
  digitalWrite(TX_LED, HIGH);
  char buf[2 + 8 * sizeof(unsigned int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    utoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(int x, int base) {
  digitalWrite(TX_LED, HIGH);
  char buf[2 + 8 * sizeof(int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    itoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(const String &s) {
  digitalWrite(TX_LED, HIGH);
  for (size_t i = 0; i < s.length(); i++) {
    putchar(s[i]);
  }
  fflush(stdout);
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(const char *p) {
  digitalWrite(TX_LED, HIGH);
  std::cout << p;
  digitalWrite(TX_LED, LOW);
}

void _Serial::print(unsigned char uc) {
  digitalWrite(TX_LED, HIGH);
  std::cout << uc;
  fflush(stdout);
  digitalWrite(TX_LED, LOW);
}

void _Serial::println() {
  _ln_flush();
}

void _Serial::println(int x) {
  print(x);
  _ln_flush();
}

void _Serial::println(int x, int base) {
  print(x, base);
  _ln_flush();
}


void _Serial::println(unsigned int x, int base) {
  print(x, base);
  _ln_flush();
}

void _Serial::println(const char *p) {
  print(p);
  _ln_flush();
}

void _Serial::println(std::string s) {
  print(s);
  _ln_flush();
}

void _Serial::println(const String &s) {
  print(s);
  _ln_flush();
}

void _Serial::println(unsigned char uc) {
  print(uc);
  _ln_flush();
}

void _Serial::write(char *p) {
  print(p);
}

void _Serial::_ln_flush() {
  std::cout << std::endl;
  fflush(stdout);
}