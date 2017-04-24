#include <Esplora.h>
void setup() {}
int count = 0;

void loop() {
  if (Esplora.readButton(1) == LOW) {
    Esplora.writeRed(255);
  }
  else {
    Esplora.writeRGB(0, 0, 0);
  }
}
