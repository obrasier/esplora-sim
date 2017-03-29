#ifndef GLOBS_H_
#define GLOBS_H_

#include "Serial.h"
#include "Device.h"
#include "Esplora.h"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>


// pin states of the arduino
extern int x_pinValue[NUM_PINS];

// led states for microbit simulator
extern std::mutex m_leds;
extern int x_leds[25]; 

// shutdown the simulator
extern std::atomic<bool> _shutdown;

// stop the arduino code running
extern std::atomic<bool> _running;

// run the simulator in fast_mode
extern std::atomic<bool> _fast_mode;

// send updates back to server or not
extern std::atomic<bool> _send_updates;

// suspend the arduino code
extern std::atomic<bool> _suspend;
extern std::mutex _m_suspend;
extern std::condition_variable _cv_suspend;

extern serial Serial;
extern serial Serial1;

extern esplora Esplora;

// 
extern std::mutex _m_device;
extern Device _device;

#endif