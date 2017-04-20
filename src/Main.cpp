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

std::mutex m_suspend;
std::condition_variable cv_suspend;

// tells the code thread to shutdown, suspend or operate in fast_mode
std::atomic<bool> shutdown(false);
std::atomic<bool> suspend(false);
std::atomic<bool> fast_mode(false);

// send updates back to the browser
std::atomic<bool> send_updates(true);
// run the student code
std::atomic<bool> running(true);

// File descriptor to write ___device_updates.
int updates_fd = -1;

// current loop number
std::atomic<uint32_t> current_loop(0);

// Elapsed time of the arduino in microseconds
uint64_t
get_elapsed_micros() {
  return _device.get_micros();
}

// Write to the output pipe
// Add a 5 us delay to stop data corruption from spamming the command line gui
void
write_to_updates(const void* buf, size_t count) {
  write(updates_fd, buf, count);
  std::this_thread::sleep_for(std::chrono::microseconds(5));
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

void print_arrays(std::array<int, NUM_PINS> pins, std::array<int, MUX_PINS> mux) {
  for (auto i : pins)
    std::cout << i << ' ';
  std::cout << std::endl;
  for (auto i : mux)
    std::cout << i << ' ';
  std::cout << std::endl;
}


void send_pin_update() {
  static int prev_pins[NUM_PINS] = {0};
  static int pins[NUM_PINS] = {0};
  static int prev_mux[MUX_PINS] = {0};
  static int mux[MUX_PINS] = {0};
  static int prev_pwm_dutycycle[NUM_PINS] = {0};
  if (!send_updates)
    return;
  int pwm_dutycycle[NUM_PINS] = {0};
  int pwm_period[NUM_PINS] = {0};
  std::array<PinState, NUM_PINS> curr_pins = _device.get_all_pins();
  std::array<int, MUX_PINS> curr_mux = _device.get_all_mux();
  for (int i = 0; i < NUM_PINS; i++) {
    if (curr_pins[i] == GPIO_PIN_OUTPUT_PWM)
      pwm_dutycycle[i] = _device.get_pwm_dutycycle(i);
  }
  memcpy(mux, curr_mux.data(), sizeof(mux));

  if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0 ||
      memcmp(mux, prev_mux, sizeof(prev_mux)) != 0 ||
      memcmp(pwm_dutycycle, prev_pwm_dutycycle, sizeof(prev_pwm_dutycycle)) != 0 ) {
    // pin states have changed
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    appendf(&json_ptr, json_end, "[{ \"type\": \"microbit_pins\", \"ticks\": %d, \"data\": {",
            get_elapsed_micros());

    list_to_json("p", &json_ptr, json_end, pins, sizeof(pins) / sizeof(int));
    appendf(&json_ptr, json_end, ", ");

    list_to_json("mux", &json_ptr, json_end, mux, sizeof(mux) / sizeof(int));
    appendf(&json_ptr, json_end, ", ");

    list_to_json("pwmd", &json_ptr, json_end, pwm_dutycycle, sizeof(pwm_dutycycle) / sizeof(int));
    appendf(&json_ptr, json_end, ", ");

    list_to_json("pwmp", &json_ptr, json_end, pwm_period, sizeof(pwm_period) / sizeof(int));
    appendf(&json_ptr, json_end, "}}]\n");

    write_to_updates(json, json_ptr - json);

    memcpy(prev_pins, pins, sizeof(pins));
    memcpy(prev_mux, mux, sizeof(mux));
    memcpy(prev_pwm_dutycycle, pwm_dutycycle, sizeof(pwm_dutycycle));
  }
}

void print_led_array() {
  std::array<int, NUM_LEDS> curr_leds = _device.get_all_leds();
  for (auto i : curr_leds)
    std::cout << i << ' ';
  std::cout << std::endl;
}

void
send_led_update() {
  static int prev_leds[NUM_LEDS] = {0};
  static int leds[NUM_LEDS] = {0};
  if (!send_updates)
    return;
  std::array<int, NUM_LEDS> curr_leds = _device.get_all_leds();
  memcpy(leds, curr_leds.data(), sizeof(leds));
  if (memcmp(leds, prev_leds, sizeof(leds)) != 0) {
    char json[1024];
    char* json_ptr = json;
    char* json_end = json + sizeof(json);
    appendf(&json_ptr, json_end, "[{ \"type\": \"microbit_leds\", \"ticks\": %d, \"data\": {",
            get_elapsed_micros());

    list_to_json("b", &json_ptr, json_end, leds, sizeof(leds) / sizeof(int));

    appendf(&json_ptr, json_end, "}}]\n");

    write_to_updates(json, json_ptr - json);
    memcpy(prev_leds, leds, sizeof(leds));
  }
}


// Write ack to say we received the data.
void
write_event_ack(const char* event_type, const char* ack_data_json) {
  char json[1024];
  char* json_ptr = json;
  char* json_end = json + sizeof(json);

  appendf(&json_ptr, json_end,
          "[{ \"type\": \"microbit_ack\", \"ticks\": %d, \"data\": { \"type\": \"%s\", \"data\": "
          "%s }}]\n",
          get_elapsed_micros(), event_type, ack_data_json ? ack_data_json : "{}");

  write_to_updates(json, json_ptr - json);
}

// Process a button event
void
process_client_button(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Button event missing id and/or state\n");
    return;
  }
  int switch_num = id->as.number;
  int val = (state->as.number == 0) ? 1 : 0;

  _device.set_mux_value(switch_num, val);
  write_event_ack("microbit_button", nullptr);
}

// Process a temperature event
void
process_client_temperature(const json_value* data) {
  const json_value* t = json_value_get(data, "t");
  if (!t || t->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Temperature event missing t\n");
    return;
  }
  _device.set_mux_value(CH_TEMPERATURE, t->as.number);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"t\": %d", static_cast<int32_t>(t->as.number));
  write_event_ack("temperature", ack_json);
}

// Process a temperature event
void
process_client_light(const json_value* data) {
  const json_value* l = json_value_get(data, "l");
  if (!l || l->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Light event missing l\n");
    return;
  }
  _device.set_mux_value(CH_LIGHT, l->as.number);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"l\": %d", static_cast<int32_t>(l->as.number));
  write_event_ack("light", ack_json);
}

// Process a slider event
void
process_client_slider(const json_value* data) {
  const json_value* s = json_value_get(data, "s");
  if (!s || s->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Slider event missing s\n");
    return;
  }
  _device.set_mux_value(CH_SLIDER, s->as.number);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"s\": %d", static_cast<int32_t>(s->as.number));
  write_event_ack("slider", ack_json);
}


// Process a slider event
void
process_client_microphone(const json_value* data) {
  const json_value* mic = json_value_get(data, "mic");
  if (!mic || mic->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Microphone event missing mic\n");
    return;
  }
  _device.set_mux_value(CH_MIC, mic->as.number);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"mic\": %d", static_cast<int32_t>(mic->as.number));
  write_event_ack("mic", ack_json);
}

// Process a accelerometer event
void
process_client_accel(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Joystick event missing id and/or state\n");
    return;
  }
  int pin_num = id->as.number;
  int val = state->as.number;
  _device.set_pin_value(pin_num, val);
  write_event_ack("accel", nullptr);
}

// Process joystick switch event
void
process_client_joystick_sw(const json_value* data) {
  const json_value* j_sw = json_value_get(data, "j_sw");
  if (!j_sw || j_sw->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Joystick switch event missing j_sw\n");
    return;
  }
  _device.set_mux_value(CH_JOYSTICK_SW, (j_sw->as.number == 0) ? 1023 : 0);
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"j_sw\": %d", static_cast<int32_t>(j_sw->as.number));
  write_event_ack("joystick_switch", ack_json);
}

// Process a joystick event. Note the joystick is 11 and 12 on
// the multiplexer, so we apply an offset of 11 to the value.
void
process_client_joystick(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Joystick event missing id and/or state\n");
    return;
  }
  int pin_num = id->as.number + CH_JOYSTICK_X;
  int val = state->as.number;
  _device.set_mux_value(pin_num, val);
  write_event_ack("joystick", nullptr);
}

void
process_client_pins(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Button event missing id and/or state\n");
    return;
  }
  int pin_num = id->as.number;
  int val = state->as.number;
  _device.set_pin_value(pin_num, val);
  write_event_ack("microbit_pin", nullptr);
}


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
        std::unique_lock<std::mutex> lk(m_suspend);
        suspend = false;
        cv_suspend.notify_all();
      } else if (strncmp(event_type->as.string, "suspend", 7) == 0) {
        std::unique_lock<std::mutex> lk(m_suspend);
        suspend = true;
      } else if (strncmp(event_type->as.string, "microbit_button", 15) == 0) {
        // Button state change.
        process_client_button(event_data);
      } else if (strncmp(event_type->as.string, "temperature", 15) == 0) {
        // Temperature change.
        process_client_temperature(event_data);
      } else if (strncmp(event_type->as.string, "accelerometer", 13) == 0) {
        // Accelerometer values change.
        process_client_accel(event_data);
      } else if (strncmp(event_type->as.string, "microbit_pin", 13) == 0) {
        // Something driving the GPIO pins.
        process_client_pins(event_data);
      } else if (strncmp(event_type->as.string, "light", 13) == 0) {
        // Light event
        process_client_light(event_data);
      } else if (strncmp(event_type->as.string, "microphone", 13) == 0) {
        // Light event
        process_client_microphone(event_data);
      } else if (strncmp(event_type->as.string, "joystick", 13) == 0) {
        // Light event
        process_client_joystick(event_data);
      } else if (strncmp(event_type->as.string, "joystick_sw", 13) == 0) {
        // Light event
        process_client_joystick_sw(event_data);
      } else if (strncmp(event_type->as.string, "slider", 13) == 0) {
        // Something driving the GPIO pins.
        process_client_slider(event_data);
      } else {
        fprintf(stderr, "Unknown event type: %s\n", event_type->as.string);
      }
      char msg_text[120];
      sprintf(msg_text, "event type: %s", event_type->as.string);
    }
    event = event->next;
  }
}

//
void
process_client_event(int fd) {
  char buf[10240];
  ssize_t len = read(fd, buf, sizeof(buf));
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
  write_event_ack("microbit_event", nullptr);
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

} // namespace

// Run the Arduino code
void
run_code() {
  setup();
  _sim::increment_counter(1032); // takes 1032 us for setup to run
  while (_sim::running) {
    _sim::current_loop++;
    loop();
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

// Setup the signal handler then run the code
void
code_thread_main() {
  // handle SIGINTs
  struct sigaction handle_sigint;
  handle_sigint.sa_handler = sig_handler;
  sigemptyset(&handle_sigint.sa_mask);
  handle_sigint.sa_flags = 0;
  sigaction(SIGINT, &handle_sigint, NULL);

  // run the code
  run_code();
}



void
main_thread() {
  const int MAX_EVENTS = 10;
  int epoll_fd = epoll_create1(0);

  // Add non-blocking stdin to epoll set.
  struct epoll_event ev_stdin;
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
  ev_stdin.events = EPOLLIN;
  ev_stdin.data.fd = STDIN_FILENO;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin);

  // Open the events pipe.
  char* client_pipe_str = getenv("GROK_CLIENT_PIPE");
  int client_fd = -1;
  int notify_fd = -1;
  int client_wd = -1;
  if (client_pipe_str != NULL) {
    client_fd = atoi(client_pipe_str);
    fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
    struct epoll_event ev_client_pipe;
    ev_client_pipe.events = EPOLLIN;
    ev_client_pipe.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client_pipe);
  } else {
    // Create and truncate the client events file.
    client_fd = open("___client_events", O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);

    // Set up a notify for the file and add to epoll set.
    struct epoll_event ev_client;
    notify_fd = inotify_init1(0);
    client_wd = inotify_add_watch(notify_fd, "___client_events", IN_MODIFY);
    if (client_wd == -1) {
      perror("add watch");
      return;
    }
    ev_client.events = EPOLLIN;
    ev_client.data.fd = notify_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, notify_fd, &ev_client) == -1) {
      perror("epoll_ctl");
      return;
    }
  }

  int epoll_timeout = 50;
  while (!_sim::shutdown) {
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, epoll_timeout);

    if (nfds == -1) {
      if (errno == EINTR) {
        // Timeout or interrupted.
        // Allow the vm branch hook to proceed.
        // Continue so that we'll catch the shutdown flag above (if it's set, otherwise continue as
        // normal).
        continue;
      }
      perror("epoll wait\n");
      exit(1);
    }

    for (int n = 0; n < nfds; ++n) {
      if (notify_fd != -1 && events[n].data.fd == notify_fd) {
        // A change occured to the ___client_events file.
        uint8_t buf[4096] __attribute__((aligned(__alignof__(inotify_event))));
        ssize_t len = read(notify_fd, buf, sizeof(buf));
        if (len == -1) {
          continue;
        }

        const struct inotify_event* event;
        for (uint8_t* p = buf; p < buf + len; p += sizeof(inotify_event) + event->len) {
          event = reinterpret_cast<inotify_event*>(p);
          if (event->wd == client_wd) {
            _sim::process_client_event(client_fd);
          }
        }
      } else if (events[n].data.fd == client_fd) {
        // A write happened to the client events pipe.
        _sim::process_client_event(client_fd);
      }
    }
  }
  if (notify_fd != -1) {
    close(notify_fd);
  }
  close(client_fd);
}

void show_help(char *s) {
  std::cout << "Usage:   " << s << " [-option] " << std::endl;
  std::cout << "option:  " << "-h  show help information" << std::endl;
  std::cout << "         " << "-d  debug mode" << std::endl;
  std::cout << "         " << "-f  fast mode" << std::endl;
  std::cout << "         " << "-v  show version infomation" << std::endl;
  exit(0);
}

int
main(int argc, char** argv) {
  // get command line options
  char tmp;
  bool debug = false;
  while ((tmp = getopt(argc, argv, "hdfv")) != -1) {
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
      case 'v':
        std::cout << "Arduino sim version is: 0.1" << std::endl;
        exit(0);
        break;
      default:
        show_help(argv[0]);
        break;
    }
  }

  // ignore SIGINTs
  struct sigaction handle_sigint;
  handle_sigint.sa_handler = SIG_IGN;
  sigemptyset(&handle_sigint.sa_mask);
  handle_sigint.sa_flags = 0;
  sigaction(SIGINT, &handle_sigint, NULL);

  // setup
  _sim::setup_output_pipe();

  // run the code
  std::thread code_thread(code_thread_main);
  code_thread.detach();

  // start the main thread to read data
  main_thread();
  close(_sim::updates_fd);
  return EXIT_SUCCESS;
}