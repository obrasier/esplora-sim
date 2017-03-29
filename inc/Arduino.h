#ifndef ARDUINO_H_
#define ARDUINO_H_

#include "binary.h"
#include "wiring.h"
#include <stdint.h>


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
//    bool - if true, spawns a new thread and returns instantly
//    variable arguments (what you feed into std::bind)
// class _Later
// {
// public:
//   template <class callable, class... arguments>
//   _Later(int after, bool async, callable&& f, arguments&&... args)
//   {
//     std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

//     if (async)
//     {
//       std::thread([after, task]() {
//         std::this_thread::sleep_for(std::chrono::milliseconds(after));
//         task();
//       }).detach();
//     }
//     else
//     {
//       std::this_thread::sleep_for(std::chrono::milliseconds(after));
//       task();
//     }
//   }
// };


void pinMode(uint8_t pin, int mode);
void digitalWrite(uint8_t pin, int value);
int digitalRead(uint8_t pin);
void analogWrite(uint8_t pin, int value); //PWM
int analogRead(uint8_t pin);
void tone(uint8_t pin, unsigned int freq);
void tone(uint8_t pin, unsigned int freq, unsigned long duration);
void noTone(uint8_t pin);
void shiftOut(int dataPin, int clockPin, int bitOrder, int value);
int shiftIn(int dataPin, int clockPin, int bitOrder);
unsigned long pulseIn(uint8_t pin, int value);
unsigned long pulseIn(uint8_t pin, int value, unsigned long timeout);
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