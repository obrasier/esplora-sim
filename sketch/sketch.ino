
void setup() {}

void loop() {
  // if (Esplora.readButton(1) == LOW && Esplora.readButton(2) == LOW) 
  //   Esplora.writeRGB(0, 0, 255);
  // else if (Esplora.readButton(1) == LOW) 
  //   Esplora.writeRGB(255, 0, 0);
  // else if (Esplora.readButton(1) == LOW) 
  //   Esplora.writeRGB(0, 255, 0);
  // else
  //   Esplora.writeRGB(0, 0, 0);
  int x = digitalRead(10);
  Serial.println(24, HEX);
  delay(1000);
}