#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "Arduino.h"
#include "Device.h"
#include "Serial.h"
#include "WString.h"
#include "ultoa.h"

void _Serial::begin(unsigned long baud_rate)
{
  // digitalMode[0] = RX;
  // digitalMode[1] = TX;
  for (int i = 0; i < 12; i++) {
    if (baud_rate == _possible_bauds[i]) {
      _baud_rate = baud_rate;
      return;
    }
  }
  _baud_rate = 9600;
}


void _Serial::end()
{
  // digitalMode[0] = FREE;
  // digitalMode[1] = FREE;

}

int _Serial::available()  // returns the number of bytes available to read
{
  return (1);
}

char _Serial::read() // the first byte of incoming serial data available (or -1 if no data is available)
{
  return (-1);
}

int _Serial::peek()
{
  return (-1);
}

void _Serial::flush()
{
}

void _Serial::print(int x)
{
  
  std::cout << x;

}

void _Serial::print(float x)
{
  std::cout << x;

}

void _Serial::print(std::string s)
{
  const char *p;
  p = s.c_str();
  std::cout << p;
  fflush(stdout);
}

void _Serial::print(unsigned int x, int base)
{
  char buf[2 + 8 * sizeof(unsigned int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    utoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
}

void _Serial::print(int x, int base)
{
  char buf[2 + 8 * sizeof(int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    itoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
}

void _Serial::print(const String &s)
{
  for (size_t i = 0; i < s.length(); i++) {
    putchar(s[i]);
  }
  fflush(stdout);
  // increment_counter(28 + (14 * s.length()));
}


void _Serial::print(const char *p)
{
  std::cout << p;
  int len = strlen(p);
  // measured on Arduino esplora 9600 baud
  // increment_counter(28 + (14 * len));
}

void _Serial::print(unsigned char uc)
{
  std::cout << uc;
  fflush(stdout);
}

void _Serial::println()
{
  _ln_flush();
}

void _Serial::println(int x)
{
  print(x);
  _ln_flush();
}

void _Serial::println(int x, int base)
{
  print(x, base);
  _ln_flush();
}


void _Serial::println(unsigned int x, int base)
{
  print(x, base);
  _ln_flush();
}

void _Serial::println(const char *p)
{
  print(p);
  _ln_flush();
}

void _Serial::println(std::string s)
{
  print(s);
  _ln_flush();
}

void _Serial::println(const String &s)
{
  print(s);
  _ln_flush();
}

void _Serial::println(unsigned char uc)
{
  print(uc);
  _ln_flush();
}

void _Serial::write(char *p)
{
  print(p);
}

void _Serial::_ln_flush()
{
  std::cout << std::endl;
  fflush(stdout);
}