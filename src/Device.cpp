/*
  Device.cpp - Arduino simulator Esplora board library
  Written by Owen Brasier

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Device.h"
#include "global_variables.h"

#include <iostream>


_Device::_Device() {
  _sim::send_pin_update();
  _sim::send_led_update();
  _clock_start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  _clock_offset_us = 0;
  _micros_elapsed = 0;
  std::array<int, 4> switches {{ CH_SWITCH_1, CH_SWITCH_2, CH_SWITCH_3, CH_SWITCH_4 }};
  zero_all_pins();
  set_led(0, MAX_LED);
  // set switches to be high (active low)
  for (const auto &elem : switches)
    set_mux_value(elem, 1023);
  set_mux_value(CH_JOYSTICK_SW, 1023);
  _sim::send_pin_update();
  _sim::send_led_update();
}

void _Device::add_offset(int64_t us) {
  _clock_offset_us += us;
}

void _Device::increment_counter(uint32_t us) {
  _micros_elapsed += us;
  std::lock_guard<std::mutex> lk(_m_countdown);
  for (int i = 0; i < NUM_PINS; i++) {
    if (_countdown[i] > 0) {
      _countdown[i] -= us;
      if (_countdown[i] <= 0) {
        _countdown[i] = 0;
        // timer has expired on pin i
        set_pin_value(_countdown[i], 0);
      }
    }
  }
}

uint64_t _Device::get_micros() {
  // sys_time<std::chrono::microseconds> clock_now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  // auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock_now - _clock_start);
  // _micros_elapsed = elapsed.count() + _clock_offset_us;
  return _micros_elapsed;
}

void _Device::set_pin_value(int pin, int value) {
  std::lock_guard<std::mutex> lk(_m_pins);
  _pin_values[pin] = value;
}

int _Device::get_pin_value(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return  _pin_values[pin];
}

void _Device::set_mux_value(int pin, int value) {
  std::lock_guard<std::mutex> lk(_m_mux);
  _mux[pin] = value;
}

int _Device::get_mux_value(int pin) {
  std::lock_guard<std::mutex> lk(_m_mux);
  return _mux[pin];
}

std::array<PinState, NUM_PINS> _Device::get_all_pins() {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _pin_states;
}

std::array<int, MUX_PINS> _Device::get_all_mux() {
  std::lock_guard<std::mutex> lk(_m_mux);
  return _mux;
}


void _Device::zero_all_pins() {
  std::lock_guard<std::mutex> lk(_m_pins);
  std::fill(_pin_values.begin(), _pin_values.end(), 0);
}


void _Device::set_pin_mode(int pin, int mode) {
  std::lock_guard<std::mutex> lk(_m_modes);
  switch (mode) {
    case INPUT:
      set_pin_state(pin, GPIO_PIN_INPUT_FLOATING);
      break;
    case INPUT_PULLUP:
      set_pin_state(pin, GPIO_PIN_INPUT_UP_HIGH);
      break;
    case OUTPUT:
      set_pin_state(pin, GPIO_PIN_OUTPUT_LOW);
      break;
    default:
      return;
  }
  _pin_modes[pin] = mode;
}

int _Device::get_pin_mode(int pin) {
  std::lock_guard<std::mutex> lk(_m_modes);
  return _pin_modes[pin];
}

void _Device::set_pin_state(int pin, PinState state) {
  std::lock_guard<std::mutex> lk(_m_states);
  _pin_states[pin] = state;
}

PinState _Device::get_pin_state(int pin) {
  std::lock_guard<std::mutex> lk(_m_states);
  return _pin_states[pin];
}

void _Device::set_pwm_dutycycle(int pin, uint32_t dutycycle) {
  std::lock_guard<std::mutex> lk(_m_pwmd);
  for (int i = 0; i < NUM_LEDS; i++) {
    if (pin == _led_map[i]) {
      int v = map(dutycycle, 0, 255, 0, MAX_LED);
      set_led(i, v);
      break;
    }
  }
  _pwm_dutycycle[pin] = dutycycle;
  set_pin_state(pin, GPIO_PIN_OUTPUT_PWM);
}

uint32_t _Device::get_pwm_dutycycle(int pin) {
  std::lock_guard<std::mutex> lk(_m_pwmd);
  return _pwm_dutycycle[pin];
}

void _Device::set_pwm_period(int pin, uint8_t period) {
  std::lock_guard<std::mutex> lk(_m_pwmp);
  _pwm_period[pin] = period;
}

uint8_t _Device::get_pwm_period(int pin) {
  std::lock_guard<std::mutex> lk(_m_pwmp);
  return  _pwm_period[pin];
}

void _Device::set_digital(int pin, int level) {
  std::lock_guard<std::mutex> lk(_m_pins);
  for (int i = 0; i < NUM_LEDS; i++) {
    if (pin == _led_map[i]) {
      set_led(i, (level == HIGH) ? MAX_LED : 0);
    }
  }
  _digital_values[pin] = level;
  if (level == LOW)
    set_pin_state(pin, GPIO_PIN_OUTPUT_LOW);
  else if (level == HIGH)
    set_pin_state(pin, GPIO_PIN_OUTPUT_HIGH);
}

int _Device::get_digital(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _digital_values[pin];
}

void _Device::set_analog(int pin, int value) {
  std::lock_guard<std::mutex> lk(_m_pins);
  if (pin >= 18) // work for pin numbers as well as channel numbers
    pin -= 18;
  if (isAnalogPin(pin))
    _analog_values[pin] = value;
}

int _Device::get_analog(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  if (pin >= 18)
    pin -= 18;
  if (isAnalogPin(pin))
    return _analog_values[pin];
  return 0;
}

void _Device::set_tone(int pin, int value) {
  set_pin_value(pin, value);
}

void _Device::set_led(int led, uint8_t brightness) {
  std::lock_guard<std::mutex> lk(_m_leds);
  _led_values[led] = brightness;
}

std::array<int, NUM_LEDS> _Device::get_all_leds() {
  std::lock_guard<std::mutex> lk(_m_leds);
  return _led_values;
}

void _Device::set_countdown(int pin, uint32_t d) {
  std::lock_guard<std::mutex> lk(_m_countdown);
  _countdown[pin] = d;
}

void _Device::set_pullup_digwrite(int pin, int value) {
  PinState mode = get_pin_state(pin);
  if (value == HIGH) {
    // enable pullup
    if (mode >= GPIO_PIN_INPUT_FLOATING && mode <= GPIO_PIN_INPUT_FLOATING_HIGH) {
      set_pin_state(pin, GPIO_PIN_INPUT_UP_HIGH);
    }
  } else if (value == LOW) {
    if (mode >= GPIO_PIN_INPUT_UP_LOW && mode <= GPIO_PIN_INPUT_DOWN_HIGH)
      set_pin_state(pin, GPIO_PIN_INPUT_FLOATING);
  }
}

bool _Device::digitalPinHasPWM(int p) {
  return ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11 || (p) == 13);
}

bool _Device::isAnalogPin(int p) {
  return ((p) >= 0 && (p) <= 11);
}
// void _Device::start_suspend() {
//   _suspend_start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
// }

// // subtract from the _clock_offset_us the duration of the code suspend in microseconds
// void _Device::stop_suspend() {
//   sys_time<std::chrono::microseconds> clock_now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
//   auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock_now - _suspend_start);
//   _clock_offset_us -= elapsed.count();
// }

namespace _sim {
// check the suspend flag, if suspend is false, then continue
// otherwise wait for the condition variable, cv_suspend
void
check_suspend() {
  std::unique_lock<std::mutex> lk(m_suspend);
  cv_suspend.wait(lk, [] {return suspend == false;});
}

// If we receive a shutdown signla
// send a final status update before
// exiting, enables fast_mode so the loop will finish
// without any delays
void
check_shutdown() {
  if (shutdown) {
    running = false;
    fast_mode = true;
    send_pin_update();
    send_led_update();
    send_updates = false;
  }
}

void
increment_counter(int us) {
  _device.increment_counter(us);
  check_suspend();
  check_shutdown();
  send_pin_update();
  send_led_update();
}
} // namespace