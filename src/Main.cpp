/*
  Arduino Simulator - An Arduino Esplora simulator
  Copyright (c) 2017 Australian Computing Academy.  All right reserved.
  Written by Owen Brasier, Jim Mussared

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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/timerfd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <chrono>
#include <cstdarg>
#include <string>
#include <array>
#include <algorithm>
#include "Arduino.h"
#include "Device.h"
#include "Serial.h"

#include "global_variables.h"



extern "C" {
#include "buffer.h"
#include "json.h"
}

// allow serial
_Serial Serial;

// Esplora
_Esplora Esplora;


// namespace _sim so the arduino code can't call
// all the functions the simulator uses easily
namespace _sim {

_Device _device;

uint64_t
get_elapsed_millis();

namespace {

// When did we last write a heartbeat, in (if enabled in heartbeat_mode).
const int32_t HEARTBEAT_US = 60000;
const int32_t MAX_SLEEP = 20000;
const int32_t UPDATE_US = 20000;

// tells the code thread to shutdown, suspend or operate in fast_mode
std::atomic<bool> shutdown(false);
std::atomic<bool> suspend(false);
std::atomic<bool> fast_mode(false);

std::atomic<bool> heartbeat_mode(false);

// send updates back to the browser
std::atomic<bool> send_updates(true);
// run the student code
std::atomic<bool> running(true);

// File descriptor to write ___device_updates.
int updates_fd = -1;
int client_fd = -1;

// current loop number
std::atomic<uint32_t> current_loop(0);

// Write to the output pipe
// Add a 5 us delay to stop data corruption from spamming the command line gui
void
write_to_updates(const void* buf, size_t count, bool should_suspend = false) {
  if (should_suspend) {
    suspend = true;
  }
  write(updates_fd, buf, count);
}

void
appendf(char** str, const char* end, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int n = vsnprintf(*str, end - *str, format, args);
  va_end(args);
  *str += min(end - *str, n);
}

// Generate json string from a list of ints.
// {1,2,3} --> '"<field>:" [1,2,3]'
void
list_to_json(const char* field, char** json_ptr, char* json_end, int* values, size_t len) {
  if (len == 0) {
    appendf(json_ptr, json_end, "\"%s\": []", field);
  } else {
    appendf(json_ptr, json_end, "\"%s\": [", field);

    for (size_t i = 0; i < len; ++i) {
      appendf(json_ptr, json_end, "%d,", values[i]);
    }

    // Replace trailing comma with ']'.
    *(*json_ptr - 1) = ']';
  }
}

void send_pin_update() {
  static int prev_pins[NUM_PINS] = {0};
  static int prev_pwm_high_time[NUM_PINS] = {0};
  static int prev_pwm_period[NUM_PINS] = {0};
  if (!send_updates)
    return;
  int pins[NUM_PINS];
  int pwm_high_time[NUM_PINS] = {0};
  int pwm_period[NUM_PINS] = {0};
  for (int i = 0; i < NUM_PINS; i++) {
    pins[i] = _device.get_pin_state(i);
    if (pins[i] == GPIO_PIN_OUTPUT_PWM) {
      pwm_high_time[i] = _device.get_pwm_high_time(i);
      pwm_period[i] = _device.get_pwm_period(i);
    }
  }

  if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0 ||
      memcmp(pwm_period, prev_pwm_period, sizeof(prev_pwm_period)) != 0 ||
      memcmp(pwm_high_time, prev_pwm_high_time, sizeof(prev_pwm_high_time)) != 0 ) {
    // pin states have changed
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    appendf(&json_ptr, json_end, "[{ \"type\": \"arduino_pins\", \"ticks\": %" PRIu64 ", \"data\": {",
            get_elapsed_millis());

    list_to_json("p", &json_ptr, json_end, pins, sizeof(pins) / sizeof(int));
    appendf(&json_ptr, json_end, ", ");

    list_to_json("pwmd", &json_ptr, json_end, pwm_high_time, sizeof(pwm_high_time) / sizeof(int));
    appendf(&json_ptr, json_end, ", ");

    list_to_json("pwmp", &json_ptr, json_end, pwm_period, sizeof(pwm_period) / sizeof(int));
    appendf(&json_ptr, json_end, "}}]\n");

    write_to_updates(json, json_ptr - json, true);

    memcpy(prev_pins, pins, sizeof(pins));
    memcpy(prev_pwm_high_time, pwm_high_time, sizeof(pwm_high_time));
    memcpy(prev_pwm_period, pwm_period, sizeof(pwm_period));
  }
}

void
check_random_updates() {
  bool exceeded = false;
  static bool exceeded_prev = false;

  exceeded = has_exceeded_random_call_limit();

  if (exceeded != exceeded_prev) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);

    appendf(&json_ptr, json_end,
            "[{ \"type\": \"random_state\", \"ticks\": %" PRIu64 ", \"data\": { \"exceeded\": %s }}]\n",
            get_elapsed_millis(), exceeded ? "true" : "false");

    write_to_updates(json, json_ptr - json, true);

    exceeded_prev = exceeded;
  }
}


void
check_marker_failure_updates() {
  const char* category = nullptr;
  const char* message = nullptr;

  bool has_failure = get_marker_failure_event(&category, &message);

  if (has_failure) {
    char json[20480];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);

    struct buffer* category_buf = buffer_create();
    json_write_escape_string(category_buf, category);
    buffer_reserve(category_buf, 1);
    category_buf->data[category_buf->nbytes_used] = 0;

    struct buffer* message_buf = buffer_create();
    json_write_escape_string(message_buf, message);
    buffer_reserve(message_buf, 1);
    message_buf->data[message_buf->nbytes_used] = 0;

    appendf(&json_ptr, json_end,
            "[{ \"type\": \"marker_failure\", \"ticks\":  %" PRIu64 ", \"data\": { \"category\": %s, "
            "\"message\": %s }}]\n",
            get_elapsed_millis(), category_buf->data, message_buf->data);

    buffer_destroy(category_buf);
    buffer_destroy(message_buf);

    write_to_updates(json, json_ptr - json, true);

    set_marker_failure_event(nullptr, nullptr);
  }
}


void
write_heartbeat() {

  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  appendf(&json_ptr, json_end,
          "[{ \"type\": \"arduino_heartbeat\", \"ticks\": %" PRIu64 ", \"data\": { \"real_ticks\": \"%" PRIu64 "\" "
          "}}]\n",
          get_elapsed_millis(), wall_time_micros() / 1000);

  write_to_updates(json, json_ptr - json, true);

}

void
write_hello() {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  appendf(&json_ptr, json_end,
          "[{ \"type\": \"arduino_hello\", \"ticks\": %" PRIu64 ", \"data\": {}}]\n",
          get_elapsed_millis());

  write_to_updates(json, json_ptr - json, false);
}

void
write_bye() {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  appendf(&json_ptr, json_end,
          "[{ \"type\": \"arduino_bye\", \"ticks\": %" PRIu64 ", \"data\": { \"real_ticks\": \"%" PRIu64 "\" }}]\n",
          get_elapsed_millis(), wall_time_micros() / 1000);

  write_to_updates(json, json_ptr - json, false);
}


// Write ack to say we received the data.
void
write_event_ack(const char* event_type, const char* ack_data_json) {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  appendf(&json_ptr, json_end,
          "[{ \"type\": \"arduino_ack\", \"ticks\": %" PRIu64 ", \"data\": { \"type\": \"%s\", \"data\": "
          "%s }}]\n",
          get_elapsed_millis(), event_type, ack_data_json ? ack_data_json : "{}");
  write_to_updates(json, json_ptr - json, false);
}

// process a multiplexer event - the pins are as follows:
// 0 - button 1
// 1 - button 2
// 2 - button 3
// 3 - button 4
// 4 - slider
// 5 - light
// 6 - temperature
// 7 - microphone
// 8 - tinkerkit A
// 9 - tinkerkit B
// 10 - joystick sw
// 11 - joystick x
// 12 - joystick y
void
process_client_mux(const json_value* data) {
  const json_value* id = json_value_get(data, "pin");
  const json_value* voltage = json_value_get(data, "voltage");
  if (!id || !voltage || id->type != JSON_VALUE_TYPE_NUMBER ||
      voltage->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Mux event missing id and/or voltage\n");
    return;
  }

  int pin_num = id->as.number;
  double v = voltage->as.number;
  _device.set_mux_voltage(pin_num, v);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"pin\": %d, \"v\": %.2f}", static_cast<int32_t>(pin_num), static_cast<double>(v));
  write_event_ack("arduino_mux", ack_json);
}

// Esplora pins
// 5 - Red
// 10 - Green
// 9 - Blue
// 23 - A5 - accel x
// 29 - A11 - accel y
// 24 - A6 - accel z
// 6 - speaker
void
process_client_pins(const json_value* data) {
  const json_value* id = json_value_get(data, "pin");
  const json_value* voltage = json_value_get(data, "voltage");
  if (!id || !voltage || id->type != JSON_VALUE_TYPE_NUMBER ||
      voltage->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Button event missing id and/or voltage\n");
    return;
  }
  int pin_num = id->as.number;
  int val = voltage->as.number;
  _device.set_pin_voltage(pin_num, val);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"pin\": %d, \"v\":%.2f}", static_cast<int32_t>(pin_num), static_cast<double>(val));
  write_event_ack("arduino_pin", ack_json);
}

// void
// process_client_random(const json_value* data) {
//   const json_value* next = json_value_get(data, "next");
//   const json_value* repeat = json_value_get(data, "repeat");
//   const json_value* choice_count = json_value_get(data, "choice_count");
//   const json_value* choice_result = json_value_get(data, "choice_result");
//   if (next && repeat && next->type == JSON_VALUE_TYPE_NUMBER &&
//       repeat->type == JSON_VALUE_TYPE_NUMBER) {
//     set_random_state(next->as.number, repeat->as.number);
//   } else if (choice_count && choice_result && choice_count->type == JSON_VALUE_TYPE_NUMBER &&
//              choice_result->type == JSON_VALUE_TYPE_STRING) {
//     set_random_choice(choice_count->as.number, choice_result->as.string);
//   } else {
//     fprintf(stderr, "Random needs (next, repeat) or (choice_count, choice_result).\n");
//     return;
//   }

//   write_event_ack("random", nullptr);
// }

// Handle an array of json events that we read from the pipe/file.
// All json events are at a minimum:
//   { "type": "<string>", "data": { <object> } }
void
process_client_json(const json_value* json) {
  if (json->type != JSON_VALUE_TYPE_ARRAY) {
    fprintf(stderr, "Client event JSON wasn't a list.\n");
  }
  const json_value_list* event = json->as.pairs;
  while (event) {
    if (event->value->type != JSON_VALUE_TYPE_OBJECT) {
      fprintf(stderr, "Event should be an object.\n");
      event = event->next;
      continue;
    }
    const json_value* event_type = json_value_get(event->value, "type");
    const json_value* event_data = json_value_get(event->value, "data");
    if (!event_type || !event_data || event_type->type != JSON_VALUE_TYPE_STRING ||
        event_data->type != JSON_VALUE_TYPE_OBJECT) {
      fprintf(stderr, "Event missing type and/or data.\n");
    } else {
      if (strncmp(event_type->as.string, "resume", 6) == 0) {
        suspend = false;
      } else if (strncmp(event_type->as.string, "suspend", 7) == 0) {
        suspend = true;
      } else if (strncmp(event_type->as.string, "arduino_pin", 13) == 0) {
        // Something driving the GPIO pins.
        process_client_pins(event_data);
      } else if (strncmp(event_type->as.string, "arduino_mux", 11) == 0) {
        // Something driving the GPIO pins.
        process_client_mux(event_data);
      } else {
        fprintf(stderr, "Unknown event type: %s\n", event_type->as.string);
      }
      char msg_text[120];
      sprintf(msg_text, "event type: %s", event_type->as.string);
    }
    event = event->next;
  }
}

// takes in a client json, and decides if it is actually a json
void
process_client_event(int fd) {
  char buf[10240];
  ssize_t len = read(fd, buf, sizeof(buf));
  if (len == -1) {
    return;
  }
  if (len == sizeof(buf)) {
    fprintf(stderr, "Too much data in client event.\n");
    return;
  }
  buf[len] = 0;

  char* line_start = buf;
  while (*line_start) {
    char* line_end = strchrnul(line_start, '\n');

    json_value* json = json_parse_n(line_start, line_end - line_start);
    if (json) {
      process_client_json(json);
      json_value_destroy(json);
    } else {
      fprintf(stderr, "Invalid JSON\n");
    }

    if (!*line_end) {
      break;
    }
    line_start = line_end + 1;
  }
}

// Setup the output pipe
void
setup_output_pipe() {
  // Open the events pipe.
  char* updates_pipe_str = getenv("GROK_UPDATES_PIPE");
  if (updates_pipe_str != NULL) {
    updates_fd = atoi(updates_pipe_str);
  } else {
    updates_fd = open("___device_updates", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  }
}


// check the suspend flag, if suspend is false, then continue
// otherwise wait for the condition variable, cv_suspend
void
check_suspend() {
  if (!fast_mode) {
    return;
  }
  while (suspend && !shutdown) {
    process_client_event(client_fd);
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
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
    send_updates = false;
  }
}

// updates checks if we need to update the device yet, and writes heartbeats for the marker
void
arduino_check_for_changes() {
  static uint32_t last_update_us = 0;
  static uint64_t last_heartbeat = 0;
  uint64_t curr_micros = get_arduino_micros();

  if (curr_micros > last_update_us + UPDATE_US) {
    send_pin_update();
    check_random_updates();
    check_marker_failure_updates();
    last_update_us = curr_micros;
  }

  // Periodically heartbeat if the '-t' flag is enabled.
  // This is useful for the marker to ensure that it sees an event at least every N ticks.
  if ((curr_micros > (last_heartbeat + HEARTBEAT_US))) {
    process_client_event(client_fd);
    last_heartbeat = curr_micros;
    if (heartbeat_mode) {
      write_heartbeat();
    }
  }
}

// Keeps track of the wall time so Arduino stays in sync in normal mode
void sleep_and_update(uint32_t us) {
  _device.increment_counter(us);
  uint64_t arduino_time = get_arduino_micros();
  uint64_t wall_time = wall_time_micros();
  if (!fast_mode) {
    while (wall_time < arduino_time) {
      std::this_thread::sleep_for(std::chrono::microseconds(5000));
      wall_time = wall_time_micros();
    }
  }
  arduino_check_for_changes();
}


} // namespace

// Public _sim methods provided by Main.cpp:

// Elapsed time of the arduino in microseconds
uint64_t
get_elapsed_millis() {
  return round(_device.get_micros() / 1000);
}

uint64_t
get_arduino_micros() {
  return _device.get_micros();
}

uint64_t
wall_time_micros() {
  static uint32_t starting_clock = 0;

  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &t);

  uint32_t real_us_ticks = (t.tv_sec * 1000000 + (t.tv_nsec / 1000));

  if (starting_clock == 0) {
    starting_clock = real_us_ticks - get_arduino_micros();
  }
  return real_us_ticks - starting_clock;
}

void force_pin_update() {
  send_pin_update();
}

// Increment "arduino time" by the specified micros.
// This is called all through Arduino.cpp/Esplora.cpp/Print.cpp to simulate operations taking time.
void
increment_counter(int us) {
  while (us > 0) {
    int d = min(MAX_SLEEP, us);
    sleep_and_update(d);
    us -= d;
  }
}

} // namespace _sim

namespace {

// Run the Arduino code
void
run_code() {
  _sim::increment_counter(1032); // takes 1032 us for setup to run
  setup();
  while (_sim::running) {
    _sim::current_loop++;
    loop();
    _sim::increment_counter(10);
    _sim::check_suspend();
    _sim::check_shutdown();
  }
}

// Handle SIGINT in the code thread to shutdown after a loop
// has finished and final device update has been sent
void
sig_handler(int s __attribute__((unused))) {
  _sim::shutdown = true;
  _sim::running = false;
}

void show_help(char *s) {
  std::cout << "Usage:   " << s << " [-option] " << std::endl;
  std::cout << "option:  " << "-h  show help information" << std::endl;
  std::cout << "         " << "-d  debug mode" << std::endl;
  std::cout << "         " << "-f  fast mode" << std::endl;
  std::cout << "         " << "-t  hearbeat mode" << std::endl;
  std::cout << "         " << "-v  show version infomation" << std::endl;
  exit(0);
}

} // namespace

int
main(int argc, char** argv) {
  // get command line options
  char tmp;
  bool debug = false;
  while ((tmp = getopt(argc, argv, "hdftv")) != -1) {
    switch (tmp) {
      case 'h':
        show_help(argv[0]);
        break;
      case 'd':
        debug = true;
        break;
      case 'f':
        _sim::fast_mode = true;
        break;
      case 't':
        _sim::heartbeat_mode = true;
        break;
      case 'v':
        std::cout << "Arduino sim version is: 0.1" << std::endl;
        exit(0);
        break;
      default:
        show_help(argv[0]);
        break;
    }
  }

  // setup updates_fd
  _sim::setup_output_pipe();

  // Let the UI know that the simulator has started (and compilation has finished).
  _sim::write_hello();

  if (_sim::heartbeat_mode) {
    _sim::write_heartbeat();
  }

  // handle SIGINTs
  struct sigaction handle_sigint;
  handle_sigint.sa_handler = sig_handler;
  sigemptyset(&handle_sigint.sa_mask);
  handle_sigint.sa_flags = 0;
  sigaction(SIGINT, &handle_sigint, NULL);


  // Set non-blocking stdin.
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);

  // Open the events pipe.
  char* client_pipe_str = getenv("GROK_CLIENT_PIPE");
  if (client_pipe_str != NULL) {
    _sim::client_fd = atoi(client_pipe_str);
    fcntl(_sim::client_fd, F_SETFL, fcntl(_sim::client_fd, F_GETFL, 0) | O_NONBLOCK);
  } else {
    // Create and truncate the client events file.
    _sim::client_fd = open("___client_events", O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);
  }

  run_code();

  _sim::write_bye();

  close(_sim::client_fd);
  close(_sim::updates_fd);

  return EXIT_SUCCESS;
}
