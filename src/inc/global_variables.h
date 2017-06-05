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
  extern _Device _device;
}

extern _Serial Serial;
// extern _Serial Serial1;

extern _Esplora Esplora;

#endif
