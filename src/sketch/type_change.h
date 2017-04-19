// Need to come up with a better way to do overflow etc.
// At the moment none exists, but I don't know a better way.... :/
#include "Arduino.h"
#include "Serial.h"
#include "Esplora.h"
#define arduino_int int
#define int int_fast16_t
