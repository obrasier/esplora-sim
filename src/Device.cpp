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


double dmap(double val, double x1, double x2, double y1, double y2) {
  return (val - x1) * (y2 - y1) / (x2 - x1) + y1;
}

_Device::_Device() {
  _micros_elapsed = 0;

  for (int i = 0; i < NUM_PINS; i++) {
    _pins[i]._pin = i;
    _pins[i]._voltage = NAN;
    if (isAnalogPin(i))
      _pins[i]._is_analog = true;
    // accelerometer pins
    if (i == 23 || i == 29 || i == 24)
      _pins[i]._voltage = 2.5;
  }

  std::array<int, 4> switches {{ CH_SWITCH_1, CH_SWITCH_2, CH_SWITCH_3, CH_SWITCH_4 }};
  // set switches to be high (active low)
  for (const auto &elem : switches)
    set_mux_voltage(elem, 5.0);
  set_mux_voltage(CH_JOYSTICK_SW, 5.0);
  set_mux_voltage(CH_TEMPERATURE, 0.75);
  set_mux_voltage(CH_LIGHT, 5.0);
  set_mux_voltage(CH_MIC, 0.0);
}


void _Device::increment_counter(uint32_t us) {
  _micros_elapsed += us;
  std::lock_guard<std::mutex> lk(_m_countdown);
  for (int i = 0; i < NUM_PINS; i++) {
    if (_pins[i]._countdown > 0) {
      _pins[i]._countdown -= us;
      if (_pins[i]._countdown <= 0) {
        _pins[i]._countdown = 0;
        // timer has expired on pin i
        set_tone(i, 0);
      }
    }
  }
}

uint64_t _Device::get_micros() {
  return _micros_elapsed;
}

void _Device::set_pin_voltage(int pin, int value) {
  std::lock_guard<std::mutex> lk(_m_pins);
  _pins[pin]._voltage = value;
}

double _Device::get_pin_voltage(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return  _pins[pin]._voltage;
}

void _Device::set_mux_voltage(int pin, double value) {
  std::lock_guard<std::mutex> lk(_m_mux);
  _mux_pins[pin]._voltage = value;
}

double _Device::get_mux_voltage(int pin) {
  std::lock_guard<std::mutex> lk(_m_mux);
  return _mux_pins[pin]._voltage;
}


int _Device::get_mux_value(int pin) {
  std::lock_guard<std::mutex> lk(_m_mux);
  return dmap(_mux_pins[pin]._voltage, 0, 5.0, 0, 1023);
}


void _Device::set_pin_mode(int pin, int mode) {
  std::lock_guard<std::mutex> lk(_m_pins);
  switch (mode) {
    case INPUT:
      _pins[pin]._state = GPIO_PIN_INPUT_FLOATING;
      set_input(pin);
      break;
    case INPUT_PULLUP:
      _pins[pin]._state = GPIO_PIN_INPUT_UP_HIGH;
      set_input(pin);
      break;
    case OUTPUT:
      _pins[pin]._state = GPIO_PIN_OUTPUT_LOW;
      set_output(pin);
      break;
    default:
      return;
  }
  _pins[pin]._mode = mode;
}

int _Device::get_pin_mode(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _pins[pin]._mode;
}

void _Device::set_pin_state(int pin, PinState state) {
  std::lock_guard<std::mutex> lk(_m_pins);
  _pins[pin]._state = state;
}

PinState _Device::get_pin_state(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _pins[pin]._state;
}

void _Device::set_pwm_dutycycle(int pin, uint32_t a_write) {
  std::lock_guard<std::mutex> lk(_m_pins);
  set_output(pin);
  if (a_write == 0) {
    _pins[pin]._state = GPIO_PIN_OUTPUT_LOW;
    _pins[pin]._pwm_period = 0;
    _pins[pin]._pwm_high_time = 0;
  } else {
    _pins[pin]._is_pwm = true;
    _pins[pin]._state = GPIO_PIN_OUTPUT_PWM;
  }
  uint32_t high_time = _pins[pin]._pwm_period * ((float)a_write / 255.0);
  _pins[pin]._pwm_high_time = high_time;
}

uint32_t _Device::get_pwm_dutycycle(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _pins[pin]._pwm_high_time;
}

void _Device::set_pwm_period(int pin, uint32_t period) {
  std::lock_guard<std::mutex> lk(_m_pins);
  _pins[pin]._pwm_period = period;
}

uint32_t _Device::get_pwm_period(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return  _pins[pin]._pwm_period;
}

void _Device::default_pwm_period(int pin) {
  for (auto const& elem : _pwm_frequencies) {
    if (elem.first == pin) {
      float freq = (1.0 / static_cast<float>(elem.second)) * 1000000.0;
      set_pwm_period(pin, static_cast<uint32_t>(freq));
    }
  }
}

void _Device::set_digital(int pin, int level) {
  std::lock_guard<std::mutex> lk(_m_pins);
  _pins[pin]._state = (level == LOW) ? GPIO_PIN_OUTPUT_LOW : GPIO_PIN_OUTPUT_HIGH;
}

int _Device::get_digital(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  Pin p = _pins[pin];
  if (isnan(p._voltage)) {
    if (p._mode == INPUT_PULLUP) {
      p._voltage = 5.0;
      return HIGH;
    } else
      return (rand() % 2 == 0) ? HIGH : LOW;
  }
  if (p._mode == INPUT)
    return (p._voltage >= 3.0) ? HIGH : LOW;
  else if (p._mode == INPUT_PULLUP)
    return (p._voltage >= 1.0) ? HIGH : LOW;
  else if (p._mode == OUTPUT)
    return (p._state == GPIO_PIN_OUTPUT_HIGH) ? HIGH : LOW;
  return (rand() % 2 == 0) ? HIGH : LOW;
}

uint32_t _Device::get_analog(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  if (pin >= 0 && pin <= 11)
    pin += 18;
  if (isnan(_pins[pin]._voltage))
    return rand() % 1024;
  if (_pins[pin]._is_analog)
    return dmap(_pins[pin]._voltage, 0, 5.0, 0, 1023);
  return rand() % 1024;
}

void _Device::set_tone(int pin, uint32_t freq) {
  uint32_t period = 0;
  _m_pins.lock();
  if (freq != 0) {
    period = 1000000 / freq;
    _pins[pin]._is_tone = true;
  }
  else {
    _pins[pin]._is_tone = false;
  }
  _m_pins.unlock();
  set_pwm_period(pin, period);
  set_pwm_dutycycle(pin, freq);
  _sim::send_pin_update();
}

bool _Device::is_tone(int pin) {
  std::lock_guard<std::mutex> lk(_m_pins);
  return _pins[pin]._is_tone;
}

void _Device::set_countdown(int pin, uint32_t d) {
  std::lock_guard<std::mutex> lk(_m_countdown);
  _pins[pin]._countdown = d;
}

void _Device::set_pullup_digwrite(int pin, int value) {
  std::lock_guard<std::mutex> lk(_m_pins);
  PinState state = _pins[pin]._state;
  if (value == HIGH) {
    // enable pullup
    if (state >= GPIO_PIN_INPUT_FLOATING && state <= GPIO_PIN_INPUT_FLOATING_HIGH)
      _pins[pin]._state = GPIO_PIN_INPUT_UP_HIGH;
  } else if (value == LOW) {
    if (state >= GPIO_PIN_INPUT_UP_LOW && state <= GPIO_PIN_INPUT_DOWN_HIGH)
      _pins[pin]._state = GPIO_PIN_INPUT_FLOATING;
  }
}

bool _Device::digitalPinHasPWM(int p) {
  return (p == 3 || p == 5 || p == 6 || p == 9 || p == 10 || p == 11 || p == 13);
}

bool _Device::isAnalogPin(int p) {
  return (p >= A0 && p <= A11);
}

void _Device::set_input(int pin) {
  _pins[pin]._is_output = false;
  _pins[pin]._is_pwm = false;
}

void _Device::set_output(int pin) {
  _pins[pin]._is_output = true;
  _pins[pin]._is_pwm = false;
}


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
    send_updates = false;
  }
}

void
increment_counter(int us) {
  _device.increment_counter(us);
  check_suspend();
  check_shutdown();
  send_pin_update();
}
} // namespace