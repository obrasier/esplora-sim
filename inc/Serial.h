#ifndef SERIAL_H_
#define SERIAL_H_

#include "WString.h"
#include "Arduino.h"
#include <string>


class serial {

public:
  void begin(unsigned long baud_rate);
  void end();
  int  available();
  char read();
  int  peek();
  int availableForWrite();
  void flush();
  void print(int x);
  void print(float x);
  void print(int x, int base);
  void print(unsigned int x, int base);
  void print(std::string s);
  void print(const char *p);
  void print(const String &s);
  void print(unsigned char c);
  void println(int x);
  void println(int x, int base);
  void println(unsigned int x, int base);
  void println(const char *p);
  void println(std::string s);
  void println(const String &s);
  void println();
  void println(unsigned char c);
  void write(char *p);
  operator bool() { return true; }

private:
  uint32_t _baud_rate = 9600;
  const uint32_t _possible_bauds[12] = {300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200};
  void _ln_flush();

};
#endif