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
  // Esplora.tone(500, 1000);
  analogWrite(3, 200);
  delay(2000);
  analogWrite(3, 0);
  delay(2000);
}
