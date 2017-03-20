#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "Arduino.h"
#include "Hardware.h"
#include "Serial.h"
#include "String.h"


void serial::begin(int baud_rate)
{
  // digitalMode[0] = RX;
  // digitalMode[1] = TX;
  _baud_rate = baud_rate;
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
//  increment_counter(100);
}

void serial::print(int x)
{
//  increment_counter(100);
  std::cout << x;

}

void print_binary(size_t const size, void const * const ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  int i, j;

  for (i = size - 1; i >= 0; i--)
  {
    for (j = 7; j >= 0; j--)
    {
      byte = (b[i] >> j) & 1;
      printf("%u", byte);
    }
  }
  puts("");
}

void serial::print(int x, int base)
{
//  increment_counter(16);
  char buf[20];
  switch (base) {
  case BIN:
    print_binary(sizeof(x), &x);
    break;
  case OCT:
    sprintf(buf, "%o", x);
    std::cout << buf;
    break;
  case DEC:
    sprintf(buf, "%d", x);
    std::cout << buf;
    break;
  case HEX:
    sprintf(buf, "%x", x);
    std::cout << buf;
    break;
  }
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

}


void serial::println(int x)
{
  increment_counter(28);
  std::cout << x << std::endl;

}

void serial::println(const char *p)
{
  increment_counter(28);
  std::cout << p << std::endl;

}

void serial::println(std::string s)
{
  increment_counter(28);
  const char *p;
  p = s.c_str();
  std::cout << p << std::endl;

}

void serial::println(String s)
{
 increment_counter(28);
  const char *p;
  p = s.getPointer();
  std::cout << p << std::endl;

}
void serial::println()
{
  increment_counter(47); // measured on Esplora
  std::cout << std::endl;
}

void serial::println(unsigned char uc)
{
  increment_counter(65);
  std::cout << uc << std::endl;
}

void serial::write(char *p)
{
  print(p);
}
