/*
  wiring.h - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/
  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#ifndef WIRING_H_
#define WIRING_H_

//#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h> // for round, coming Arduino 1.8.3
#include <cmath>
#include "binary.h"


#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#else
#define INTERNAL 3
#endif
#define DEFAULT 1
#define EXTERNAL 0


#define  F_CPU  16000000

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

template <typename T, typename S>
inline T min(T a, S b) {
  return ((a) < (b) ? (a) : (b));
}
template <typename T, typename S>
inline T max(T a, S b) {
  return ((a) > (b) ? (a) : (b));
}

template<typename T>
inline T abs (T x) {
  return (x > 0) ? x : -x;
}

template<typename T, typename S>
inline T constrain(T amt, S low, S high) {
  return (amt < low) ? low : (amt > high) ? high : amt;
}

// template<typename T>
// inline T round(T x) {
//   return std::round(x);
// }

template<typename T>
inline T radians(T deg) {
  return deg * DEG_TO_RAD;
}

template<typename T>
inline T degrees(T rad) {
  return rad * RAD_TO_DEG;
}

template<typename T>
T sq(T x) {
  return x * x;
}


typedef unsigned int word;


void init(void);
void done(void);

void analogReference(uint8_t mode);

#define interrupts() sei()
#define noInterrupts() cli()

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

// void setup(void);
// void loop(void);

int* empty(void);

extern void fdevopen(int (*)(char, FILE*), int);


#define SPDR (*empty())
#define SPCR (*empty())
#define SPSR (*empty())
#define SPIE (*empty())
#define SPIF (*empty())
#define _BV(x) (1<<(x))


#endif