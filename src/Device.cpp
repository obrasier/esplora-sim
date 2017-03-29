#include "Device.h"
#include "global_variables.h"

#include <iostream>

_Device::_Device() {

}

void _Device::set_pin_value(int pin, int value) {
  _m_pins.lock();
  _pin_values[pin] = value;
  _m_pins.unlock();
}

int _Device::get_pin_value(int pin) {
  _m_pins.lock();
  int val =  _pin_values[pin];
  _m_pins.unlock();
  return val;
}

void _Device::set_mux_value(int pin, int value) {
  std::cout << "setting pin " << pin << " : " << value << std::endl;
  _m_mux.lock();
  _mux[pin] = value;
  _m_mux.unlock();
}

int _Device::get_mux_value(int pin) {
  _m_mux.lock();
  int m = _mux[pin];
  _m_mux.unlock();
  return m;
}

std::array<int, NUM_PINS> _Device::get_all_pins() {
  _m_pins.lock();
  std::array<int, NUM_PINS> p = _pin_values;
  _m_pins.unlock();
  return p;
}

std::array<int, MUX_PINS> _Device::get_all_mux() {
  _m_mux.lock();
  std::array<int, MUX_PINS> m = _mux;
  _m_mux.unlock();
  return m;
}


void _Device::zero_all_pins() {
  _m_pins.lock();
  std::fill(_pin_values.begin(), _pin_values.end(), 0);
  _m_pins.unlock();
}


void _Device::set_pin_mode(int pin, int mode) {
  _m_modes.lock();
  _pin_modes[pin] = mode;
  _m_modes.unlock();
}

int _Device::get_pin_mode(int pin) {
  _m_modes.lock();
  int p = _pin_modes[pin];
  _m_modes.unlock();
  return p;
}

void _Device::set_pwm_dutycycle(int pin, uint32_t dutycycle) {
  _m_device.lock();
  _pwm_dutycycle[pin] = dutycycle;
  _m_device.unlock();
}

uint32_t _Device::get_pwm_dutycycle(int pin) {
  _m_device.lock();
  uint32_t duty = _pwm_dutycycle[pin]; 
  _m_device.unlock();
  return duty;
}

void _Device::set_pwm_period(int pin, uint8_t period) {
  _m_device.lock();
  _pwm_period[pin] = period;
  _m_device.unlock();
}

uint8_t _Device::get_pwm_period(int pin) {
  _m_device.lock();
  uint8_t period =  _pwm_period[pin];
  _m_device.unlock();
  return period;
}

void _Device::set_digital(int pin, int level) {
  _m_device.lock();
  _digital_states[pin] = level;
  _m_device.unlock();
}

int _Device::get_digital(int pin) {
  _m_device.lock();
  int state =  _digital_states[pin];
  _m_device.unlock();
  return state;
}

void _Device::set_analog(int pin, int value) {
  _m_device.lock();
  _analog_states[pin] = value;
  _m_device.unlock();
}

int _Device::get_analog(int pin) {
  _m_device.lock();
  int val =  _analog_states[pin];
  _m_device.unlock();
  return val;
}

void _Device::set_led(int led, uint8_t brightness) {
  _m_leds.lock();
  _led_values[led] = brightness;
  _m_leds.unlock();
}

std::array<int, NUM_LEDS> _Device::get_all_leds() {
  _m_leds.lock();
  std::array<int, NUM_LEDS> a = _led_values;
  _m_leds.unlock();
  return a;
}

void _Device::increment_counter(uint32_t us) {
  _m_micros.lock();
  _micros_elapsed += us;
  _m_micros.unlock();
}

uint64_t _Device::get_micros() {
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
