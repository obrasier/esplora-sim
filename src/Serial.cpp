#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "Arduino.h"
#include "Device.h"
#include "Serial.h"
#include "WString.h"
#include "ultoa.h"

void serial::begin(unsigned long baud_rate)
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


void serial::end()
{
  // digitalMode[0] = FREE;
  // digitalMode[1] = FREE;

}

int serial::available()  // returns the number of bytes available to read
{
//  increment_counter(12);
  return (1);
}

char serial::read() // the first byte of incoming serial data available (or -1 if no data is available)
{
//  increment_counter(100);
  return (-1);
}

int serial::peek()
{
//  increment_counter(100);
  return (-1);
}

void serial::flush()
{
}

void serial::print(int x)
{
//  increment_counter(100);
  std::cout << x;

}

void serial::print(float x)
{
//  increment_counter(100);
  std::cout << x;

}

void serial::print(std::string s)
{
  increment_counter(28);
  const char *p;
  p = s.c_str();
  std::cout << p;
  fflush(stdout);
}

void serial::print(unsigned int x, int base)
{
//  increment_counter(16);
  char buf[2 + 8 * sizeof(unsigned int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    utoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
}

void serial::print(int x, int base)
{
//  increment_counter(16);
  char buf[2 + 8 * sizeof(int)];
  if (base == BIN || base == OCT || base == DEC || base == HEX) {
    itoa(x, buf, base);
    std::cout << buf;
    fflush(stdout);
  }
}

void serial::print(const String &s)
{
  for (size_t i = 0; i < s.length(); i++) {
    putchar(s[i]);
  }
  fflush(stdout);
  increment_counter(28 + (14 * s.length()));
}


void serial::print(const char *p)
{
  std::cout << p;
  int len = strlen(p);
  // measured on Arduino esplora 9600 baud
  increment_counter(28 + (14 * len));
}

void serial::print(unsigned char uc)
{
  increment_counter(28);
  std::cout << uc;
  fflush(stdout);
}

void serial::println()
{
  increment_counter(28); // measured on Esplora
  _ln_flush();
}

void serial::println(int x)
{
  print(x);
  _ln_flush();
}

void serial::println(int x, int base)
{
  print(x, base);
  _ln_flush();
}


void serial::println(unsigned int x, int base)
{
  print(x, base);
  _ln_flush();
}

void serial::println(const char *p)
{
  print(p);
  _ln_flush();
}

void serial::println(std::string s)
{
  print(s);
  _ln_flush();
}

void serial::println(const String &s)
{
  print(s);
  _ln_flush();
}

void serial::println(unsigned char uc)
{
  print(uc);
  _ln_flush();
}

void serial::write(char *p)
{
  print(p);
}

void serial::_ln_flush()
{
  std::cout << std::endl;
  fflush(stdout);
}