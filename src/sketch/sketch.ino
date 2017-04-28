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
  Esplora.tone(500, 1000);
  int x = Esplora.readLightSensor();
//  Serial.println(Esplora.readTemperature(DEGREES_C));
//  Serial.println(Esplora.readLightSensor());
//  Serial.println(Esplora.readAccelerometer(X_AXIS));
//  Serial.println(Esplora.readAccelerometer(Y_AXIS));
//  Serial.println(Esplora.readAccelerometer(Z_AXIS));
//  Serial.println(Esplora.readJoystickSwitch());
//  Serial.println(Esplora.readJoystickButton());
//  Serial.println(Esplora.readButton(SWITCH_1));
//  Serial.println(Esplora.readButton(SWITCH_2));
//  Serial.println(Esplora.readButton(3));
//  Serial.println(Esplora.readButton(4));
//  Serial.println(Esplora.readMicrophone());
  delay(2000);
}
