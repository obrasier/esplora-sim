#include <Esplora.h>
void setup() { }
void loop() {
  if (Esplora.readButton(1) == LOW) {
    Esplora.writeRed(255);
  }
  else {
    Esplora.writeRed(0);
  }
}
