#ifndef DEVICE_H_
#define DEVICE_H_

#include <array>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <algorithm>
#include <vector>
#include <random>
#include "pins_arduino.h"
#include "wiring.h"

#define NUM_PINS            31
#define MUX_PINS            13
#define NUM_LEDS            25
#define NUM_ANALOG_PINS     12

enum PinState {
  GPIO_PIN_OUTPUT_LOW = 0,
  GPIO_PIN_OUTPUT_HIGH, 
  GPIO_PIN_OUTPUT_PWM,
  GPIO_PIN_INPUT_FLOATING,
  GPIO_PIN_INPUT_FLOATING_LOW,
  GPIO_PIN_INPUT_FLOATING_HIGH,
  GPIO_PIN_INPUT_UP_LOW,
  GPIO_PIN_INPUT_UP_HIGH,
  GPIO_PIN_INPUT_DOWN_LOW,
  GPIO_PIN_INPUT_DOWN_HIGH,
  GPIO_PIN_RESERVED,
};

namespace _sim {
void check_suspend();
void check_shutdown();
void increment_counter(int us);

// in Main.cpp
void send_pin_update();
void send_led_update();


}


struct Pin {
  uint32_t _pin;
  bool _is_output = false;
  bool _is_analog = false;
  PinState _state = GPIO_PIN_OUTPUT_LOW;
  uint8_t _mode = INPUT;
  float _voltage = NAN;
  bool _is_pwm = false;
  uint32_t _dutycylce = 0;
  uint32_t _value = 0;
  int64_t _countdown = 0;
};


// The device class stores all the information required about a device.
// It is designed to be thread-safe internally, so no external mutexes should
// be required.
class _Device {
 private:

  std::atomic<uint64_t> _micros_elapsed;

  std::array<int, NUM_PINS> _pin_values;
  std::array<PinState, NUM_PINS> _pin_states;
  std::array<int, NUM_PINS> _pin_modes;
  std::array<int, NUM_LEDS> _led_values;
  std::array<int, NUM_PINS> _pwm_dutycycle;
  std::array<int, NUM_PINS> _pwm_period;
  std::array<int, MUX_PINS> _mux;

  std::array<uint, NUM_ANALOG_PINS> _analog_values;
  std::array<uint, NUM_PINS> _digital_values;

  std::array<int, NUM_PINS> _countdown;

  std::array<Pin, NUM_PINS> _pins;
  std::array<Pin, MUX_PINS> _mux_pins;

  std::array<void (*)(void), 5> _isr_table;

  std::mutex _m_device;
  std::mutex _m_pins;
  std::mutex _m_modes;
  std::mutex _m_states;
  std::mutex _m_leds;
  std::mutex _m_mux;
  std::mutex _m_pwm;
  std::mutex _m_analog;
  std::mutex _m_countdown;

  const uint8_t _tx = 2;
  const int MAX_LED = 255;

  std::array<int, 5> _interrupt_map = {{0, 1, 2, 3, 7}};

 public:
  _Device();
  void set_pin_value(int pin, int value);
  int get_pin_value(int pin);
  void set_mux_voltage(int pin, double value);
  int get_mux_value(int pin);
  std::array<PinState, NUM_PINS> get_all_pins();
  std::array<int, MUX_PINS> get_all_mux();
  void zero_all_pins();

  void set_tone(int pin, int value);
  // pin_mode holds the what is called to pinMode
  void set_pin_mode(int pin, int mode);
  int get_pin_mode(int pin);

  // pin_state holds the pin_state for the marker
  void set_pin_state(int pin, PinState state);
  PinState get_pin_state(int pin);
  void set_pwm_dutycycle(int pin, uint32_t dutycycle);
  uint32_t get_pwm_dutycycle(int pin);
  void set_pwm_period(int pin, uint8_t period);
  uint8_t get_pwm_period(int pin);
  void set_digital(int pin, int level);
  int get_digital(int pin);
  void set_analog(int pin, int value);
  int get_analog(int pin);
  std::array<int, NUM_LEDS> get_all_leds();
  void set_led(int led, uint8_t brightness);
  void increment_counter(uint32_t us);
  uint64_t get_micros();
  void tx_led_on();
  void tx_led_off();
  void start_suspend();
  void stop_suspend();
  void set_countdown(int pin, uint32_t d);
  void set_pullup_digwrite(int pin, int value);
  bool digitalPinHasPWM(int p);
  bool isAnalogPin(int p);
};



#endif