#include <Esplora.h>
void setup() {}
int count = 0;

void loop() {
  Serial.println("tone on");
  Esplora.tone(500, 1000);
  delay(2000);
}
