#ifndef DEVICE_H_
#define DEVICE_H_

#include <array>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <algorithm>
#include <vector>
#include <utility>
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

// in Main.cpp:
// Advance "arduino time" by this many micros. Call this from any Arduino/Esplora API.
void increment_counter(int us);
// Force an immediate flush of pin/led state.
void force_pin_update();
// Timing where state is owned by Main.cpp.
uint64_t get_elapsed_millis();
uint64_t get_arduino_micros();
uint64_t wall_time_micros();

// in Device.cpp:
bool has_exceeded_random_call_limit();
void set_random_choice(int32_t count, const char* result);
void set_random_state(int32_t next, int32_t repeat);
bool get_marker_failure_event(const char** category, const char** message);
void set_marker_failure_event(const char* category, const char* message);
}

struct MPin {
  uint32_t _pin;
  bool _is_analog = true;
  float _voltage = 2.5;
};

struct Pin {
  uint32_t _pin;
  bool _is_output = false;
  bool _output = false;
  bool _is_analog = false;
  PinState _state = GPIO_PIN_OUTPUT_LOW;
  uint8_t _mode = NAN;
  float _voltage = NAN;
  bool _is_pwm = false;
  uint32_t _pwm_period = 0;
  uint32_t _pwm_high_time = 0;
  int64_t _countdown = 0;
  bool _is_tone = false;
};


// The device class stores all the information required about a device.
// It is designed to be thread-safe internally, so no external mutexes should
// be required.
class _Device {
 private:

  std::atomic<uint64_t> _micros_elapsed;
  std::atomic<uint32_t> _micros_since_heartbeat;
  std::atomic<uint32_t> _us_since_sync;


  std::array<Pin, NUM_PINS> _pins;
  std::array<MPin, MUX_PINS> _mux_pins;

  std::array<void (*)(void), 5> _isr_table;

  std::mutex _m_pins;
  std::mutex _m_mux;
  std::mutex _m_countdown;

  std::array<int, 5> _interrupt_map = {{0, 1, 2, 3, 7}};
  std::array<std::pair<int, int>, 7> _pwm_frequencies = {{ {3, 980}, {5, 490}, {6, 490}, {9,490}, {10,490}, {11,490}, {13,980} }};

  double get_voltage(int pin);
  void set_input(int pin);
  void set_output(int pin);
  void process_countdown(uint32_t us);

 public:
  _Device();
  void set_pin_voltage(int pin, int value);
  double get_pin_voltage(int pin);
  void set_mux_voltage(int pin, double value);
  int get_mux_value(int pin);
  double get_mux_voltage(int pin);

  void set_tone(int pin, uint32_t value);
  bool is_tone(int pin);
  // pin_mode holds the what is called to pinMode
  void set_pin_mode(int pin, int mode);
  int get_pin_mode(int pin);

  // pin_state holds the pin_state for the marker
  void set_pin_state(int pin, PinState state);
  PinState get_pin_state(int pin);

  void set_pwm_high_time(int pin, uint32_t dutycycle);
  uint32_t get_pwm_high_time(int pin);
  void set_pwm_period(int pin, uint32_t period);
  uint32_t get_pwm_period(int pin);
  void default_pwm_period(int pin);

  void set_digital(int pin, int level);
  int get_digital(int pin);
  uint32_t get_analog(int pin);
  void increment_counter(uint32_t us);
  uint64_t get_micros();
  void start_suspend();
  void stop_suspend();
  void set_countdown(int pin, uint32_t d);
  void set_pullup_digwrite(int pin, int value);
  bool digitalPinHasPWM(int p);
  bool isAnalogPin(int p);
};



#endif
