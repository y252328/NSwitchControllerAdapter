#include <NintendoExtensionCtrl.h>
#include <SoftwareSerial.h>

SoftwareSerial SSerial(10, 11); // RX, TX
ClassicController classic;
uint8_t buf[7];

void setup() {
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
  SSerial.begin(38400);
  classic.begin();
  buf[0] = 0;
  buf[1] = 0;
  while (!classic.connect()) {
    // Serial.println("Classic Controller not detected!");
    delay(200);
  }

}

void loop() {
  boolean success = classic.update();
  digitalWrite(13, success);
  if (!success) {
    // Serial.println("Controller disconnected!");
    buf[1] = 1 << 7;
    classic.reconnect();
    delay(200);
  } else {
    // classic.printDebug();
    buf[0] = 0;
    buf[0] |= ((classic.dpadUp()?1:0) << 7);
    buf[0] |= ((classic.dpadLeft()?1:0) << 6);
    buf[0] |= ((classic.dpadRight()?1:0) << 5);
    buf[0] |= ((classic.dpadDown()?1:0) << 4);
    buf[0] |= ((classic.buttonA()?1:0) << 3);
    buf[0] |= ((classic.buttonB()?1:0) << 2);
    buf[0] |= ((classic.buttonX()?1:0) << 1);
    buf[0] |= ((classic.buttonY()?1:0));

    buf[1] = 0;
    buf[1] |= ((classic.buttonL()?1:0) << 6);
    buf[1] |= ((classic.buttonZL()?1:0) << 5);
    buf[1] |= ((classic.buttonR()?1:0) << 4);
    buf[1] |= ((classic.buttonZR()?1:0) << 3);
    buf[1] |= ((classic.buttonPlus()?1:0) << 2);
    buf[1] |= ((classic.buttonMinus()?1:0) << 1);
    buf[1] |= ((classic.buttonHome()?1:0));

    buf[2] = classic.leftJoyX();
    buf[3] = classic.leftJoyY();
    buf[4] = classic.rightJoyY();
    buf[5] = classic.rightJoyX();
    buf[6] = buf[0] ^ buf[1];
  }
  
  if(SSerial.available()) {
    while(SSerial.available()) SSerial.read();
    SSerial.write(buf, 7);
  }
}
