#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include "Hardware.h"
#include "global_variables.h"

int x_pinValue[50] = {0};
int x_leds[25] = {0};


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
increment_counter(int us)
{
  m_elapsed.lock();
  micros_elapsed += us;
  m_elapsed.unlock();
  check_suspend();
  check_shutdown();
  if (!fast_mode)
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
