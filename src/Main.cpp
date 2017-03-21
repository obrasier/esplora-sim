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
#include "Hardware.h"
#include "Serial.h"

#include "global_variables.h"
#include "sketch.ino"

using std::string;
using std::ostream;
using std::istream;
using std::stringstream;


extern "C" {
#include "buffer.h"
#include "json.h"
}

std::mutex m_pins;
std::mutex m_leds;


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

// allow serial
serial Serial;

// current loop number
uint32_t current_loop = 0;

// mutex to protect the micros_elapsed variable
std::mutex m_elapsed;
uint64_t micros_elapsed = 0;

// to get appendf to work
inline int
min(int a, int b)
{
  return (a < b ? a : b);
}

// Elapsed time of the arduino in microseconds
uint64_t
get_elapsed_micros() {
  m_elapsed.lock();
  uint64_t e = micros_elapsed;
  m_elapsed.unlock();
  return e;
}

// Write to the output pipe
// Add a 5 us delay to stop data corruption from spamming the command line gui
void
write_to_updates(const void* buf, size_t count, bool should_suspend = false) {
  write(updates_fd, buf, count);
  // std::this_thread::sleep_for(std::chrono::microseconds(5));
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

    for (int i = 0; i < len; ++i) {
      appendf(json_ptr, json_end, "%d,", values[i]);
    }

    // Replace trailing comma with ']'.
    *(*json_ptr - 1) = ']';
  }
}


void send_pin_update() {
  if (send_updates) {
    static int prev_pins[MAX_TOTAL_PINS] = {0};
    static int pins[MAX_TOTAL_PINS] = {0};
    int pwm_dutycycle[MAX_TOTAL_PINS] = {0};
    int pwm_period[MAX_TOTAL_PINS] = {0};
    m_pins.lock();
    memcpy(pins, x_pinValue, sizeof(x_pinValue));
    m_pins.unlock();

    if (memcmp(pins, prev_pins, sizeof(prev_pins)) != 0) {
      // pin states have changed
      char json[1024];
      char* json_ptr = json;
      char* json_end = json + sizeof(json);
      appendf(&json_ptr, json_end, "[{ \"type\": \"microbit_pins\", \"ticks\": %d, \"data\": {",
              get_elapsed_micros());

      list_to_json("p", &json_ptr, json_end, pins, sizeof(pins) / sizeof(int));

      appendf(&json_ptr, json_end, ", ");
      list_to_json("pwmd", &json_ptr, json_end, pwm_dutycycle,
                   sizeof(pwm_dutycycle) / sizeof(int));

      appendf(&json_ptr, json_end, ", ");
      list_to_json("pwmp", &json_ptr, json_end, pwm_period, sizeof(pwm_period) / sizeof(int));

      appendf(&json_ptr, json_end, "}}]\n");

      write_to_updates(json, json_ptr - json, true);

      memcpy(prev_pins, pins, sizeof(pins));
    }
  }
}

void
send_led_update() {
  if (send_updates) {
    static int prev_leds[25] = {0};
    static int leds[25] = {0};
    m_leds.lock();
    memcpy(leds, x_leds, sizeof(x_leds));
    m_leds.unlock();
    if (memcmp(x_leds, prev_leds, sizeof(x_leds)) != 0) {
      char json[1024];
      char* json_ptr = json;
      char* json_end = json + sizeof(json);
      appendf(&json_ptr, json_end, "[{ \"type\": \"microbit_leds\", \"ticks\": %d, \"data\": {",
              get_elapsed_micros());

      list_to_json("b", &json_ptr, json_end, x_leds, sizeof(x_leds) / sizeof(int));

      appendf(&json_ptr, json_end, "}}]\n");

      write_to_updates(json, json_ptr - json, true);
      memcpy(prev_leds, x_leds, sizeof(x_leds));
    }
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

  write_to_updates(json, json_ptr - json, false);
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

  m_pins.lock();
  x_pinValue[switch_num+1] = val; // offset of 1 for microbit_sim
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[SIM_TEMPERATURE] = t->as.number;
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[SIM_LIGHT] = l->as.number;
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[SIM_SLIDER] = s->as.number;
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[SIM_MIC] = mic->as.number;
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[pin_num] = val;
  m_pins.unlock();
  write_event_ack("accel", nullptr);
}

void
process_client_joystick_sw(const json_value* data) {
  const json_value* j_sw = json_value_get(data, "j_sw");
  if (!j_sw || j_sw->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Joystick switch event missing j_sw\n");
    return;
  }
  m_pins.lock();
  x_pinValue[SIM_JOYSTICK_SW] = (j_sw->as.number == 0) ? 1023 : 0;
  m_pins.unlock();
  char ack_json[1024];
  snprintf(ack_json, sizeof(ack_json), "{\"j_sw\": %d", static_cast<int32_t>(j_sw->as.number));
  write_event_ack("joystick_switch", ack_json);
}

void
process_client_joystick(const json_value* data) {
  const json_value* id = json_value_get(data, "id");
  const json_value* state = json_value_get(data, "state");
  if (!id || !state || id->type != JSON_VALUE_TYPE_NUMBER ||
      state->type != JSON_VALUE_TYPE_NUMBER) {
    fprintf(stderr, "Joystick event missing id and/or state\n");
    return;
  }
  int pin_num = id->as.number;
  int val = state->as.number;
  m_pins.lock();
  x_pinValue[pin_num] = val;
  m_pins.unlock();
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
  m_pins.lock();
  x_pinValue[pin_num] = val;
  m_pins.unlock();
  write_event_ack("microbit_pin", nullptr);
}


void
process_client_random(const json_value* data) {
  // TODO

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
      } else if (strncmp(event_type->as.string, "random", 13) == 0) {
        // Injected random data (from the marker only).
        process_client_random(event_data);
      } else {
        fprintf(stderr, "Unknown event type: %s\n", event_type->as.string);
      }
      char msg_text[120];
      sprintf(msg_text, "event type: %s", event_type->as.string);
      std::cout << msg_text << std::endl;
    }
    event = event->next;
  }
}

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

// Sets the default state of the Esplora pins
// Everything is 0 except for the switches, which
// are all active low.
// TODO: set default state of slider, possibly sending
// a slider event on startup based on it's position.
void
set_esplora_state() {
  int i;
  m_pins.lock();
  for (i = 0; i < MAX_TOTAL_PINS; i++)
    x_pinValue[i] = 0;
  // set switches to be high (active low)
  for (i = 1; i < 5; i++)
    x_pinValue[i] = HIGH;
  x_pinValue[SIM_JOYSTICK_SW] = 1023;
  m_pins.unlock();
  send_pin_update();
}


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





// Run the Arduino code
void 
run_code()
{
  setup();
  increment_counter(1032); // takes 1032 us for setup to run
  while (running) {
    current_loop++;
    loop();
  }
}

void
sig_handler(int s) {
  std::cout << "\nCaught signal " << s << std::endl;
  shutdown = true;
  running = false;
}

void
code_thread_main() {
  // handle SIGINTs
  struct sigaction handle_sigint;
  handle_sigint.sa_handler = sig_handler;
  sigemptyset(&handle_sigint.sa_mask);
  handle_sigint.sa_flags = 0;
  sigaction(SIGINT, &handle_sigint, NULL);
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
  while (!shutdown) {
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
            process_client_event(client_fd);
          }
        }
      } else if (events[n].data.fd == client_fd) {
        // A write happened to the client events pipe.
        process_client_event(client_fd);
      }
    }
  }
  if (notify_fd != -1) {
    close(notify_fd);
  }

  close(client_fd);
}


int
main(int argc, char** argv) {

  // ignore SIGINTs
  struct sigaction handle_sigint;
  handle_sigint.sa_handler = SIG_IGN;
  sigemptyset(&handle_sigint.sa_mask);
  handle_sigint.sa_flags = 0;
  sigaction(SIGINT, &handle_sigint, NULL);

  setup_output_pipe();
  set_esplora_state();
  std::thread code_thread(code_thread_main);  // run the code
  code_thread.detach();
  main_thread();                    // start reading client data
  close(updates_fd);
  return EXIT_SUCCESS;
}