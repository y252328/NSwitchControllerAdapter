
#include <JoystickSwitch.h>
#include <NintendoExtensionCtrl.h>

#define modePin MOSI
#define enJSBtnPin 12
#define holdZLPin  11
#define remotePin  7

class RemoteController {
public:
    RemoteController(): req(64) {}
    void begin() {
      Serial1.begin(38400);
    }

    bool update() {
      do {
        Serial1.write(req);
        size_t s = Serial1.readBytes(buf, 7);
      } while(buf[6] != (buf[0] ^ buf[1]));
      return (buf[1] & 0b10000000) == 0;
    }
    
    bool dpadUp()   { return buf[0] & 0b10000000; }
    bool dpadLeft() { return buf[0] & 0b01000000; }
    bool dpadRight(){ return buf[0] & 0b00100000; }
    bool dpadDown() { return buf[0] & 0b00010000; }
    bool buttonA(){ return buf[0] & 0b00001000; }
    bool buttonB(){ return buf[0] & 0b00000100; }
    bool buttonX(){ return buf[0] & 0b00000010; }
    bool buttonY(){ return buf[0] & 0b00000001; }
    
    bool buttonL()    { return buf[1] & 0b01000000; }
    bool buttonZL()   { return buf[1] & 0b00100000; }
    bool buttonR()    { return buf[1] & 0b00010000; }
    bool buttonZR()   { return buf[1] & 0b00001000; }
    bool buttonPlus() { return buf[1] & 0b00000100; }
    bool buttonMinus(){ return buf[1] & 0b00000010; }
    bool buttonHome() { return buf[1] & 0b00000001; }

    int leftJoyX() { return buf[2]; }
    int leftJoyY() { return buf[3]; }
    int rightJoyY() { return buf[4]; }
    int rightJoyX() { return buf[5]; }

    void printDebug() {
      
        const char fillCharacter = '_';
      
        char buffer[62];
      
        char dpadLPrint = dpadLeft() ? '<' : fillCharacter;
        char dpadUPrint = dpadUp() ? '^' : fillCharacter;
        char dpadDPrint = dpadDown() ? 'v' : fillCharacter;
        char dpadRPrint = dpadRight() ? '>' : fillCharacter;
      
        char aButtonPrint = buttonA() ? 'A' : fillCharacter;
        char bButtonPrint = buttonB() ? 'B' : fillCharacter;
        char xButtonPrint = buttonX() ? 'X' : fillCharacter;
        char yButtonPrint = buttonY() ? 'Y' : fillCharacter;
      
        char plusPrint = buttonPlus() ? '+' : fillCharacter;
        char minusPrint = buttonMinus() ? '-' : fillCharacter;
        char homePrint = buttonHome() ? 'H' : fillCharacter;
      
        char ltButtonPrint = buttonL() ? 'X' : fillCharacter;
        char rtButtonPrint = buttonR() ? 'X' : fillCharacter;
      
        char zlButtonPrint = buttonZL() ? 'L' : fillCharacter;
        char zrButtonPrint = buttonZR() ? 'R' : fillCharacter;
      
        Serial.print("Classic ");
        sprintf(buffer,
          "%c%c%c%c | %c%c%c | %c%c%c%c L:(%2u, %2u) R:(%2u, %2u) | LT:%2u%c RT:%2u%c Z:%c%c",
          dpadLPrint, dpadUPrint, dpadDPrint, dpadRPrint,
          minusPrint, homePrint, plusPrint,
          aButtonPrint, bButtonPrint, xButtonPrint, yButtonPrint,
          leftJoyX(), leftJoyY(), rightJoyX(), rightJoyY(),
          0, ltButtonPrint, 0, rtButtonPrint,
          zlButtonPrint, zrButtonPrint);
        Serial.println(buffer);
    }

private:
    uint8_t buf[7];
    const uint8_t req;
};

RemoteController remoteClassic;
ClassicController classic;
bool mashDownMode = false;
int mashValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(modePin, INPUT);
  pinMode(remotePin, INPUT);
  pinMode(enJSBtnPin, INPUT_PULLUP);
  pinMode(holdZLPin, INPUT_PULLUP);
  Joystick.begin(false, digitalRead(modePin));
  remoteClassic.begin();
  classic.begin();
//  while (!remoteClassic.connect()) {
////    Serial.println("Classic Controller not detected!");
//    delay(1000);
//  }
}

int rightStick(uint8_t input) {
  double output_d = input;
  output_d = (output_d - 16) * 10.78;
  int output = output_d;
  if (output > 127)
    return 127;
  else if (output < -127)
    return -127;
  else
    return output;
}

int leftStick(uint8_t input) {
  double output_d = input;
  output_d = (output_d - 33) * 5.39; // min 4.9
  int output = output_d;
  if (output > 127)
    return 127;
  else if (output < -127)
    return -127;
  else
    return output;
}
bool zl_status = false;

void loop() {
  if (digitalRead(remotePin))
    remote_loop();
  else
    local_loop();
  delay(1);
}

void remote_loop() {
  boolean success = remoteClassic.update();
  digitalWrite(13, success);
  Joystick.setSwitchMode(digitalRead(modePin));
  if (!success) {
//    Serial.println("Controller disconnected!");
    delay(100);
  } else {
//    remoteClassic.printDebug();
    Joystick.setButton(2, remoteClassic.buttonA());
    Joystick.setButton(1, remoteClassic.buttonB());
    Joystick.setButton(3, remoteClassic.buttonX());
    Joystick.setButton(0, remoteClassic.buttonY());

    zl_status = remoteClassic.buttonZL() || (!digitalRead(holdZLPin) && zl_status && !remoteClassic.buttonL());
    if (!digitalRead(enJSBtnPin) && remoteClassic.buttonL() && remoteClassic.buttonZL()) {
      Joystick.setButton(4, false);
      Joystick.setButton(6, false);
      Joystick.setButton(10, true);
    } else {
      Joystick.setButton(4, remoteClassic.buttonL());
      Joystick.setButton(6, zl_status);
      Joystick.setButton(10, false);
    }
    if (!digitalRead(enJSBtnPin) && remoteClassic.buttonR() && remoteClassic.buttonZR()) {
      Joystick.setButton(5, false);
      Joystick.setButton(7, false);
      Joystick.setButton(11, true);
    } else {
      Joystick.setButton(5, remoteClassic.buttonR());
      Joystick.setButton(7, remoteClassic.buttonZR());
      Joystick.setButton(11, false);
    }


    if (remoteClassic.buttonPlus() && remoteClassic.buttonMinus()) {
      Joystick.setButton(8, false);
      Joystick.setButton(9, false);
      Joystick.setButton(13, true);
    } else {
      Joystick.setButton(9, remoteClassic.buttonPlus());
      Joystick.setButton(8, remoteClassic.buttonMinus());
      Joystick.setButton(13, false);
    }
    Joystick.setButton(12, remoteClassic.buttonHome());

    Joystick.setXAxis(leftStick(remoteClassic.leftJoyX()));
    Joystick.setYAxis(leftStick(remoteClassic.leftJoyY()+4) * -1);
    Joystick.setZAxis(rightStick(remoteClassic.rightJoyX()));
    Joystick.setZAxisRotation(rightStick(remoteClassic.rightJoyY()) * -1);

    if (remoteClassic.dpadUp()) {
      if (remoteClassic.dpadLeft())
        Joystick.setHatSwitch(7);
      else if (remoteClassic.dpadRight())
        Joystick.setHatSwitch(1);
      else
        Joystick.setHatSwitch(0);
    } else if (remoteClassic.dpadDown()) {
      if (remoteClassic.dpadLeft())
        Joystick.setHatSwitch(5);
      else if (remoteClassic.dpadRight())
        Joystick.setHatSwitch(3);
      else
        Joystick.setHatSwitch(4);
    } else if (remoteClassic.dpadRight())
      Joystick.setHatSwitch(2);
    else if (remoteClassic.dpadLeft())
      Joystick.setHatSwitch(6);
    else
      Joystick.setHatSwitch(-1);
  }

  Joystick.sendState();
}

void local_loop() {
  boolean success = classic.update();
  digitalWrite(13, success);
  Joystick.setSwitchMode(digitalRead(modePin));
  if (!success) {
//    Serial.println("Controller disconnected!");
    classic.reconnect();
    delay(100);
  } else {
    classic.printDebug();
    Joystick.setButton(2, classic.buttonA());
    Joystick.setButton(1, classic.buttonB());
    Joystick.setButton(3, classic.buttonX());
    Joystick.setButton(0, classic.buttonY());

    zl_status = classic.buttonZL() || (!digitalRead(holdZLPin) && zl_status && !classic.buttonL());
    if (!digitalRead(enJSBtnPin) && classic.buttonL() && classic.buttonZL()) {
      Joystick.setButton(4, false);
      Joystick.setButton(6, false);
      Joystick.setButton(10, true);
    } else {
      Joystick.setButton(4, classic.buttonL());
      Joystick.setButton(6, zl_status);
      Joystick.setButton(10, false);
    }
    if (!digitalRead(enJSBtnPin) && classic.buttonR() && classic.buttonZR()) {
      Joystick.setButton(5, false);
      Joystick.setButton(7, false);
      Joystick.setButton(11, true);
    } else {
      Joystick.setButton(5, classic.buttonR());
      Joystick.setButton(7, classic.buttonZR());
      Joystick.setButton(11, false);
    }


    if (classic.buttonPlus() && classic.buttonMinus()) {
      Joystick.setButton(8, false);
      Joystick.setButton(9, false);
      Joystick.setButton(13, true);
    } else {
      Joystick.setButton(9, classic.buttonPlus());
      Joystick.setButton(8, classic.buttonMinus());
      Joystick.setButton(13, false);
    }
    Joystick.setButton(12, classic.buttonHome());

    Joystick.setXAxis(leftStick(classic.leftJoyX()));
    Joystick.setYAxis(leftStick(classic.leftJoyY()+4) * -1);
    Joystick.setZAxis(rightStick(classic.rightJoyX()));
    Joystick.setZAxisRotation(rightStick(classic.rightJoyY()) * -1);

    if (classic.dpadUp()) {
      if (classic.dpadLeft())
        Joystick.setHatSwitch(7);
      else if (classic.dpadRight())
        Joystick.setHatSwitch(1);
      else
        Joystick.setHatSwitch(0);
    } else if (classic.dpadDown()) {
      if (classic.dpadLeft())
        Joystick.setHatSwitch(5);
      else if (classic.dpadRight())
        Joystick.setHatSwitch(3);
      else
        Joystick.setHatSwitch(4);
    } else if (classic.dpadRight())
      Joystick.setHatSwitch(2);
    else if (classic.dpadLeft())
      Joystick.setHatSwitch(6);
    else
      Joystick.setHatSwitch(-1);
  }

  Joystick.sendState();
}
