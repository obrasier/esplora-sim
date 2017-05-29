#include <Arduino.h>
#include <Esplora.h>
void setup() {
  pinMode(13, OUTPUT);
}
int count = 0;

void loop() {
  digitalWrite(13, HIGH);
  Serial.println(count++);
  digitalWrite(13, LOW);
  digitalWrite(13, HIGH);
  digitalWrite(13, LOW);
  delay(1000);
  // Esplora.writeRGB(255, 0, 0);
}
