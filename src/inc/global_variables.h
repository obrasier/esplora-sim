#ifndef GLOBS_H_
#define GLOBS_H_

#include "Serial.h"
#include "Device.h"
#include "Esplora.h"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>


namespace _sim {
// shutdown the simulator
extern std::atomic<bool> shutdown;

// stop the arduino code running
extern std::atomic<bool> running;

// run the simulator in fast_mode
extern std::atomic<bool> fast_mode;

// send updates back to server or not
extern std::atomic<bool> send_updates;

// suspend the arduino code
extern std::atomic<bool> suspend;
extern std::mutex m_suspend;
extern std::condition_variable cv_suspend;
extern _Device _device;

extern uint32_t time_since_sleep;
extern uint32_t last_sleep_ms;

} 

extern _Serial Serial;
// extern _Serial Serial1;

extern _Esplora Esplora;

// 

#endif