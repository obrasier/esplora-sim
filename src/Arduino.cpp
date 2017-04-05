#include <iostream>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "Arduino.h"
#include "global_variables.h"

const int sleep_period = 100;
const int us_sleep_period = sleep_period*1000;

void pinMode(int pin, int mode) {
  if (pin > NUM_PINS || pin <= 0)
    return;
  _device.set_pin_mode(pin, mode);
}

void digitalWrite(int pin, byte value) {
  if (pin > NUM_PINS || pin <= 0)
    return;
  _device.set_digital(pin, (value) ? HIGH : LOW);
  _device.set_pin_value(pin, value);
  send_pin_update();
}

int digitalRead(int pin) {
  if (pin > NUM_PINS || pin <= 0)
    return 0;
  return _device.get_pin_value(pin);
}

void analogWrite(int pin, byte value) {
  if (pin > NUM_PINS || pin <= 0)
    return;
  pinMode(pin, OUTPUT);
  _device.set_pin_value(pin, value);
  _device.set_pwm_dutycycle(pin, value);
  send_pin_update();
}

int analogRead(int pin) {
  if (pin > NUM_PINS || pin <= 0)
    return 0;
  return _device.get_pin_value(pin);
}

//------ Advanced I/O ----------------------
void tone(unsigned int pin, unsigned int freq) {
  if (pin > NUM_PINS || pin <= 0)
    return;
  _device.set_pin_value(pin, freq);
  send_pin_update();
}

void tone(unsigned int pin, unsigned int freq, unsigned long duration) {
  if (pin > NUM_PINS || pin <= 0)
    return;
  tone(pin, freq);
  _Later turn_off_tone(duration, true, &noTone, pin);
}

void noTone(unsigned int pin) {
  tone(pin, 0);
}

void shiftOut(int dataPin, int clockPin, int bitOrder, int value) {
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
}

int shiftIn(int dataPin, int clockPin, int bitOrder) {
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
  return 0;
}


//------ Time ------------------------------

unsigned long millis() {
  unsigned long e = _device.get_micros();
  return e / 1000;
}

// Return microseconds rounded down
// to the nearest multiple of 4
unsigned long
micros() {
  unsigned long e = _device.get_micros();
  int rem = e % 4;
  if (rem == 0)
    return e;
  return e - rem;
}

void delay(uint32_t ms) {
  if (ms == 0)
    return;
  if (_fast_mode) {
    _device.add_offset(ms * 1000UL);
  }
  else {
    int d = ms/sleep_period;
    int r = ms % sleep_period;
    for (int i = 0; i < d; i++) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_period));
      check_suspend();
      check_shutdown();
    }
    if (r) {
      std::this_thread::sleep_for(std::chrono::milliseconds(r));
      check_suspend();
      check_shutdown();
    }
  }
}

void delayMicroseconds(uint32_t us) {
  if (us == 0)
    return;
  if (_fast_mode) {
    _device.add_offset(us);
  }
  else {
    int d = us/us_sleep_period;
    int r = us % us_sleep_period;
    for (int i = 0; i < d; i++) {
      std::this_thread::sleep_for(std::chrono::microseconds(us_sleep_period));
      check_suspend();
      check_shutdown();
    }
    if (r) {
      std::this_thread::sleep_for(std::chrono::microseconds(r));
      check_suspend();
      check_shutdown();
    }
  }
}

int map(int x, int fromLow, int fromHigh, int toLow, int toHigh) {
  int y;
  y = (float)(x - fromLow) / (fromHigh - fromLow) * (toHigh - toLow) + toLow;
  return (y);
}


//------ Random Numbers --------------------
void randomSeed(int seed) {
  srand(seed);
}

long random(long upperLimit) {
  long x = RAND_MAX / upperLimit;
  x = long(rand() / x);
  return x;
}

long random(long lowerLimit, long upperLimit)
{
  long interval, temp = 0;
  if (lowerLimit < upperLimit) {
    interval = upperLimit - lowerLimit;
    temp = lowerLimit + random(interval);
  }
  return temp;
}

