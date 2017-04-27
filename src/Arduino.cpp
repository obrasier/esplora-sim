#include <iostream>
#include "Arduino.h"
#include "global_variables.h"

const int sleep_period = 100; // sleep period in milliseconds
const int us_sleep_period = sleep_period * 1000;


void pinMode(int pin, int mode) {
  if (pin > NUM_PINS || pin < 0)
    return;
  if (mode == INPUT || mode == INPUT_PULLUP || mode == OUTPUT) {
    _sim::_device.set_pin_mode(pin, mode);
    _sim::increment_counter(1);
  }
}

void digitalWrite(int pin, byte value) {
  if (pin > NUM_PINS || pin < 0)
    return;
  int mode = _sim::_device.get_pin_mode(pin);
  if (mode == INPUT) {
    // if mode is input on digital write, we enable (high) or disable (low)
    // the pullup resistor
    _sim::_device.set_pullup_digwrite(pin, value);
  } else if (mode == OUTPUT) {
    _sim::_device.set_digital(pin, (value) ? HIGH : LOW);
    _sim::increment_counter(1);
  }
}

int digitalRead(int pin) {
  _sim::increment_counter(1);
  return _sim::_device.get_digital(pin);
}

void analogWrite(int pin, byte value) {
  if (!_sim::_device.digitalPinHasPWM(pin))
    return;
  pinMode(pin, OUTPUT);
  if (value == 0)
    digitalWrite(pin, LOW);
  else if (value == 255)
    digitalWrite(pin, HIGH);
  else {
    _sim::_device.set_pwm_dutycycle(pin, value);
    _sim::_device.default_pwm_period(pin);
    _sim::increment_counter(1);
  }
}

int analogRead(int pin) {
  _sim::increment_counter(100);
  if (pin >= 0 && pin <= 11) pin += 18;
  return _sim::_device.get_analog(pin);
}

void analogReference(uint8_t mode __attribute__((unused))) {
  // do nothing
}

//------ Advanced I/O ----------------------
void tone(unsigned int pin, unsigned int freq) {
  if (!_sim::_device.digitalPinHasPWM(pin))
    return;
  pinMode(pin, OUTPUT);
  _sim::_device.set_tone(pin, freq);
  _sim::increment_counter(1);
}

void tone(unsigned int pin, unsigned int freq, unsigned long duration) {
  if (!_sim::_device.digitalPinHasPWM(pin))
    return;
  tone(pin, freq);
  // _sim::_Later turn_off_tone(duration, true, &noTone, pin);
  _sim::_device.set_countdown(pin, duration*1000UL);
}

void noTone(unsigned int pin) {
  tone(pin, 0);
}

void shiftOut(int dataPin __attribute__((unused)), int clockPin __attribute__((unused)), int bitOrder __attribute__((unused)), int value __attribute__((unused))) {
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
}

int shiftIn(int dataPin __attribute__((unused)), int clockPin __attribute__((unused)), int bitOrder __attribute__((unused))) {
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
  return 0;
}


//------ Time ------------------------------

unsigned long millis() {
  unsigned long e = _sim::_device.get_micros();
  return e / 1000;
}

// Return microseconds rounded down
// to the nearest multiple of 4
unsigned long
micros() {
  unsigned long e = _sim::_device.get_micros();
  int rem = e % 4;
  if (rem == 0)
    return e;
  return e - rem;
}

void delay(uint32_t ms) {
  if (ms == 0)
    return;
  if (!_sim::fast_mode) {
    // d is the division, number of sleep periods + r, remainder
    int d = ms / sleep_period;
    int r = ms % sleep_period;
    while (!_sim::fast_mode && d) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_period));
      _sim::increment_counter(sleep_period*1000UL);
      d--;
    }
    if (!_sim::fast_mode && r) {
      std::this_thread::sleep_for(std::chrono::milliseconds(r));
      _sim::increment_counter(r*1000UL);
    }
  }
}

void delayMicroseconds(uint32_t us) {
  if (us == 0)
    return;
  if (!_sim::fast_mode) {
    // d is the division, number of sleep periods + r, remainder
    int d = us / us_sleep_period;
    int r = us % us_sleep_period;
    while (!_sim::fast_mode && d) {
      std::this_thread::sleep_for(std::chrono::microseconds(us_sleep_period));
      _sim::increment_counter(us_sleep_period);
      d--;
    }
    if (!_sim::fast_mode && r) {
      std::this_thread::sleep_for(std::chrono::microseconds(r));
      _sim::increment_counter(r);
    }
  }
}

int map(int x, int fromLow, int fromHigh, int toLow, int toHigh) {
  int y;
  y = (float)(x - fromLow) / (fromHigh - fromLow) * (toHigh - toLow) + toLow;
  return y;
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

long random(long lowerLimit, long upperLimit) {
  long interval, temp = 0;
  if (lowerLimit < upperLimit) {
    interval = upperLimit - lowerLimit;
    temp = lowerLimit + random(interval);
  }
  return temp;
}

