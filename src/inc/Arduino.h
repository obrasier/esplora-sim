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

#include <stdint.h>
#include <thread>
#include <chrono>
#include "binary.h"
#include "wiring.h"
#include "pins_arduino.h"


#define HIGH 0x1
#define LOW  0x0


#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

typedef uint8_t byte;
typedef bool boolean;

// WCharacter uses boolean
#include "WCharacter.h"

// Class to run code after n milliseconds, used for the tone duration timer
// Arguments:
//    int - milliseconds until code is run
//    bool - if true, spawns a new thread and continues executing code
//    variable arguments (what you feed into std::bind)

namespace _sim {
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
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(after));
      task();
    }
  }
};
} // namespace

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

uint8_t lowByte(int x);
uint8_t highByte(int x);

int bitRead(int x, int n);
void bitSet(unsigned int *x, int n);
void bitClear(unsigned int *x, int n);
void bitWrite(unsigned int *x, int n, int b);

void interrupts();
void noInterrupts();

#define digitalPinToPCICR(p)    ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) 
#define digitalPinToPCMSK(p)    ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCMSK0) : ((uint8_t *)0))
#define digitalPinToPCMSKbit(p) {(((p) >= 8 && (p) <= 11) ? (p) - 4 : ((p) == 14 ? 3 : ((p) == 15 ? 1 : ((p) == 16 ? 2 : ((p) == 17 ? 0 : (p - A8 + 4))))))}





#define digitalPinToInterrupt(p) ((p) == 0 ? 2 : ((p) == 1 ? 3 : ((p) == 2 ? 1 : ((p) == 3 ? 0 : ((p) == 7 ? 4 : NOT_AN_INTERRUPT)))))


#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )
#define microsecondsToClockCycles(a) ( ((a) * (F_CPU / 1000L)) / 1000L )

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


namespace _sim {
void send_pin_update();
void send_led_update();
void increment_counter(int us);
}


// in sketch.ino
void setup();
void loop();

#endif