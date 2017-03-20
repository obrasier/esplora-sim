#ifndef SERIAL_H_
#define SERIAL_H_

#include "String.h"

class serial {

public:
  void begin(int baud_rate);
  void end();
  int  available();
  char read();
  int  peek();
  void flush();
  void print(int x);
  void print(int x, int base);
  void print(const char *p);
  void print(unsigned char c);
  void println(int x);
  void println(const char *p);
  void println(std::string p);
  void println(String p);
  void println();
  void println(unsigned char c);
  void write(char *p);

private:
  uint32_t _baud_rate;

};

#endif