#ifndef ARDUINO_H_
#define ARDUINO_H_

#include "binary.h"

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define HIGH 0x1
#define LOW  0x0


#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

typedef uint8_t byte;
// in Ardino.cpp

void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);
void analogWrite(int pin, int value); //PWM
int analogRead(int pin);
void tone(int pin, unsigned int freq);
void tone(int pin, unsigned int freq, unsigned long duration);
void noTone(int pin);
void shiftOut(int dataPin, int clockPin, int bitOrder, int value);
int shiftIn(int dataPin, int clockPin, int bitOrder);
unsigned long pulseIn(int pin, int value);
unsigned long pulseIn(int pin, int value, unsigned long timeout);
unsigned long millis();
unsigned long micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
int map(int x, int fromLow, int fromHigh, int toLow, int toHigh);
void randomSeed(int seed);
long random(long upperLimit);
long random(long lowerLimit, long upperLimit);
unsigned char lowByte(int x);
unsigned char highByte(int x);
int bitRead(int x, int n);
void  bitSet(unsigned int *x, int n);
void bitClear(unsigned int *x, int n);
void bitWrite(unsigned int *x, int n, int b);
int bit(int n);
void interrupts();
void noInterrupts();

// in Hardware.cpp
void increment_counter(int us);
#endif