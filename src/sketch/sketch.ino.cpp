#include <Arduino.h>
#include <Esplora.h>
void setup() {
  pinMode(13, OUTPUT);
}
int count = 0;

void loop() {
  Serial.print("printing something");
  digitalWrite(13, HIGH);
  Serial.println("printing another thing");
  delay(1000);
  Serial.print("delaaayy");
  digitalWrite(13, LOW);
  delay(1000);
  Esplora.writeRGB(255, 0, 0);
}
