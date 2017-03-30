/*
  Arduino.h - Character utility functions for Wiring & Arduino
  Copyright (c) 2017 Australian Computing Academy.  All right reserved.
  
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

#ifndef ARDUINO_H_
#define ARDUINO_H_

#include "binary.h"
#include "wiring.h"
#include <stdint.h>
#include <thread>
#include <chrono>


#define HIGH 0x1
#define LOW  0x0


#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

typedef uint8_t byte;
typedef bool boolean;

// Class to run code after n milliseconds, used for the tone duration timer
// Arguments:
//    int - milliseconds until code is run
//    bool - if true, spawns a new thread and continues executing code 
//    variable arguments (what you feed into std::bind)
class _Later {
public:
  template <class callable, class... arguments>
  _Later(int after, bool async, callable&& f, arguments&&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

    if (async) {
      std::thread([after, task]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        task();
      }).detach();
    }
    else {
      std::this_thread::sleep_for(std::chrono::milliseconds(after));
      task();
    }
  }
};


void pinMode(int pin, int mode);
void digitalWrite(int pin, byte value);
int digitalRead(int pin);
void analogWrite(int pin, byte value); //PWM
int analogRead(int pin);
void tone(unsigned int pin, unsigned int freq);
void tone(unsigned int pin, unsigned int freq, unsigned long duration);
void noTone(unsigned int pin);
void shiftOut(int dataPin, int clockPin, int bitOrder, int value);
int shiftIn(int dataPin, int clockPin, int bitOrder);
// unsigned long pulseIn(uint8_t pin, int value);
// unsigned long pulseIn(uint8_t pin, int value, unsigned long timeout);
unsigned long millis();
unsigned long micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
int map(int x, int fromLow, int fromHigh, int toLow, int toHigh);
void randomSeed(int seed);
long random(long upperLimit);
long random(long lowerLimit, long upperLimit);
// unsigned char lowByte(int x);
// unsigned char highByte(int x);
// int bitRead(int x, int n);
// void  bitSet(unsigned int *x, int n);
// void bitClear(unsigned int *x, int n);
// void bitWrite(unsigned int *x, int n, int b);
// int bit(int n);
// void interrupts();
// void noInterrupts();

// in Hardware.cpp
void increment_counter(int us);

// in sketch.ino
void setup();
void loop();

#endif