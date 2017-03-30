#ifndef DEVICE_H_
#define DEVICE_H_

#include <array>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <algorithm>

#define SIM_RED             5
#define SIM_GREEN           9
#define SIM_BLUE            10
#define SIM_ACCEL_X         23
#define SIM_ACCEL_Y         29
#define SIM_ACCEL_Z         24
#define SIM_BUZZER          6

#define NUM_PINS            30
#define MUX_PINS            13
#define NUM_LEDS            25
#define NUM_ANALOG_PINS     12


void check_suspend();
void check_shutdown();
void increment_counter(int us);

// in Main.cpp
void send_pin_update();
void send_led_update();


// The device class stores all the information required about a device.
// It is designed to be thread-safe internally, so no external mutexs should
// be required.
class _Device
{
private:
  template <class Duration>
    using sys_time = std::chrono::time_point<std::chrono::system_clock, Duration>;

  sys_time<std::chrono::microseconds> _clock_start;

  std::atomic<uint32_t> _clock_offset_us;
  std::atomic<uint64_t> _micros_elapsed;

  std::array<int, NUM_PINS> _pin_values;
  std::array<int, NUM_LEDS> _led_values;
  std::array<int, NUM_PINS> _digital_states;
  std::array<int, NUM_ANALOG_PINS> _analog_states;
  std::array<int, NUM_PINS> _pin_modes;
  std::array<int, NUM_PINS> _pwm_dutycycle;
  std::array<int, NUM_PINS> _pwm_period;
  std::array<int, MUX_PINS> _mux;
  
  std::mutex _m_device;
  std::mutex _m_pins;
  std::mutex _m_modes;
  std::mutex _m_leds;
  std::mutex _m_mux;
  const uint8_t _tx = 2;

public:
  _Device();
  void add_offset(uint32_t _us);
  void set_pin_value(int pin, int value);
  int get_pin_value(int pin);
  void set_mux_value(int pin, int value);
  int get_mux_value(int pin);
  std::array<int, NUM_PINS> get_all_pins();
  std::array<int, MUX_PINS> get_all_mux();
  void zero_all_pins();
  void set_pin_mode(int pin, int mode);
  int get_pin_mode(int pin);
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
};


#endif