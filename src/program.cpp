#include "program.h"
#include <Esplora.h>
void setup() {}
int16_t count = 0;

void loop() {
  if (Esplora.readButton(1) == LOW) {
    Esplora.writeRed(127);
  }
  else {
    Esplora.writeRGB(0, 0, 0);
  }
  int16_t x = my_func();
  Serial.println(x);
  delay(2000);

}

int16_t my_func () {
  return 4;
}

