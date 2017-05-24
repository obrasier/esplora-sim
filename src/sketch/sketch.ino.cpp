#include <Arduino.h>
#include <Esplora.h>
void setup() {
  pinMode(13, OUTPUT);
}
int count = 0;

void loop() {
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
  Serial.print("loooped\r\n");
  Esplora.writeRGB(255, 0, 0);
}
