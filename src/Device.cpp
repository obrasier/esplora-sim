#include "Device.h"
#include "global_variables.h"

#include <iostream>

Device::Device() {
}

void Device::set_pin_value(int pin, int value) {
  _m_pins.lock();
  _pin_values[pin] = value;
  _m_pins.unlock();
}

int Device::get_pin_value(int pin) {
  _m_pins.lock();
  int val =  _pin_values[pin];
  _m_pins.unlock();
  return val;
}

std::array<int, NUM_PINS> Device::get_all_pins() {
  _m_pins.lock();
  std::array<int, NUM_PINS> p = _pin_values;
  _m_pins.unlock();
  return p;
}

void Device::zero_all_pins() {
  _m_pins.lock();
  std::fill(_pin_values.begin(), _pin_values.end(), 0);
  _m_pins.unlock();
}


void Device::set_pin_mode(int pin, PinState mode) {
  _m_modes.lock();
  _pin_modes[pin] = mode;
  _m_modes.unlock();
}

PinState Device::get_pin_mode(int pin) {
  _m_modes.lock();
  PinState p = _pin_modes[pin];
  _m_modes.unlock();
  return p;
}

void Device::set_pwm_dutycycle(int pin, uint32_t dutycycle) {
  _m_device.lock();
  _pwm_dutycycle[pin] = dutycycle;
  _m_device.unlock();
}

uint32_t Device::get_pwm_dutycycle(int pin) {
  _m_device.lock();
  uint32_t duty = _pwm_dutycycle[pin]; 
  _m_device.unlock();
  return duty;
}

void Device::set_pwm_period(int pin, uint8_t period) {
  _m_device.lock();
  _pwm_period[pin] = period;
  _m_device.unlock();
}

uint8_t Device::get_pwm_period(int pin) {
  _m_device.lock();
  uint8_t period =  _pwm_period[pin];
  _m_device.unlock();
  return period;
}

void Device::set_digital(int pin, bool level) {
  _m_device.lock();
  _digital_states[pin] = level;
  _m_device.unlock();
}

bool Device::get_digital(int pin) {
  _m_device.lock();
  bool state =  _digital_states[pin];
  _m_device.unlock();
  return state;
}

void Device::set_analog(int pin, int value) {
  _m_device.lock();
  _analog_states[pin] = value;
  _m_device.unlock();
}

int Device::get_analog(int pin) {
  _m_device.lock();
  int val =  _analog_states[pin];
  _m_device.unlock();
  return val;
}

void Device::set_led(int led, uint8_t brightness) {
  _m_leds.lock();
  _led_values[led] = brightness;
  _m_leds.unlock();
}

std::array<int, NUM_LEDS> Device::get_all_leds() {
  // std::vector<int> rgb = _rgb.get_rgb();
  // std::vector<int> leds = _leds;
  // leds.insert(leds.end(), rgb.begin(), rgb.end());
  _m_leds.lock();
  std::array<int, NUM_LEDS> a = _led_values;
  _m_leds.unlock();
  return a;
}

void Device::increment_counter(uint32_t us) {
  _m_micros.lock();
  _micros_elapsed += us;
  _m_micros.unlock();
}

uint64_t Device::get_micros() {
  _m_micros.lock();
  uint64_t m = _micros_elapsed;
  _m_micros.unlock();
  return m;
}


// check the suspend flag, if suspend is false, then continue
// otherwise wait for the condition variable, cv_suspend
void
check_suspend() {
  std::unique_lock<std::mutex> lk(_m_suspend);
  _cv_suspend.wait(lk, [] {return _suspend == false;});
}

// If we receive a shutdown signla
// send a final status update before
// exiting, enables fast_mode so the loop will finish
// without any delays
void
check_shutdown() {
  if (_shutdown) {
    _running = false;
    _fast_mode = true;
    send_pin_update();
    send_led_update();
    _send_updates = false;
  }
}

void
increment_counter(int us)
{
  _device.increment_counter(us);
  check_suspend();
  check_shutdown();
  if (!_fast_mode)
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
