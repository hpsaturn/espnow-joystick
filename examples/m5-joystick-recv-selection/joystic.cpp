#include <Arduino.h>
#include <Wire.h>
#include <M5Unified.h>
#include <EspNowJoystick.hpp>

extern const unsigned char connect_on[800];
extern const unsigned char connect_off[800];

uint16_t AngleBuff[4];
uint32_t btnPowerOffcount = 0;
uint32_t suspendCount = 0;

bool receiverConnected = false;
float receiverBattVolt = 0.0;
uint16_t receiverBattLevel = 0;
uint_least32_t heartBeatStamp = 0;

EspNowJoystick joystick;
JoystickMessage jm;

uint8_t I2CRead8bit(uint8_t Addr) {
  Wire.beginTransmission(0x38);
  Wire.write(Addr);
  Wire.endTransmission();
  Wire.requestFrom(0x38, 1);
  return Wire.read();
}

uint16_t I2CRead16bit(uint8_t Addr) {
  uint16_t ReData = 0;
  Wire.beginTransmission(0x38);
  Wire.write(Addr);
  Wire.endTransmission();
  Wire.requestFrom(0x38, 2);
  for (int i = 0; i < 2; i++) {
    ReData <<= 8;
    ReData |= Wire.read();
  }
  return ReData;
}

class MyTelemetryCallbacks : public EspNowTelemetryCallbacks {
  void onTelemetryMsg(TelemetryMessage tm) {
    receiverBattVolt = tm.btv;
    receiverBattLevel = tm.btl;
    receiverConnected = tm.e1;
    suspendCount = 0;
    heartBeatStamp = millis();
  };
  void onError(const char *msg){};
};

// void drawValues(uint8_t ax, uint8_t ay, uint8_t az) {
//   Disbuff.fillRect(0, 30, 80, 130, BLACK);
//   Disbuff.setTextColor(BLUE);
//   Disbuff.setCursor(10, 26);
//   // Disbuff.printf("JOYSTICK:");
//   // Disbuff.setTextColor(WHITE);
//   // Disbuff.setCursor(10, 42);
//   // Disbuff.printf("AX: %04d", ax);
//   // Disbuff.setCursor(10, 54);
//   // Disbuff.printf("AY: %04d", ay);
//   // Disbuff.setCursor(10, 66);
//   // Disbuff.printf("AZ: %04d", az);
//   // Disbuff.setCursor(10, 82);
//   Disbuff.printf("Volt: %.2fv", M5.Axp.GetBatVoltage());
//   Disbuff.drawLine(0, 99, 80, 99, WHITE);
//   Disbuff.setCursor(10, 106);
//   Disbuff.setTextColor(MAGENTA);
//   Disbuff.printf("RECEIVER:");
//   Disbuff.setTextColor(WHITE);
//   Disbuff.setCursor(10, 122);
//   Disbuff.printf("Batt: %03d%%", receiverBattLevel);
//   Disbuff.setCursor(10, 134);
//   Disbuff.printf("BatV: %.2fv", receiverBattVolt);
// }

void updateDisplay(uint8_t ax, uint8_t ay, uint8_t az) {
  static uint_least32_t guiTimeStamp = 0;
  if (millis() - guiTimeStamp > 100) {
    guiTimeStamp = millis();
    // if (receiverConnected)
    //   Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
    // else
    //   Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_off);
    // Disbuff.pushSprite(0, 0);
    // drawValues(ax, ay, az);
    M5.update();
    if (millis() - heartBeatStamp > 1000) {
      receiverConnected = false;  // heartbeat should be renew this flag
    }
  }
}

void setup() {
  auto cfg = M5.config();

  M5.begin(cfg);
  /// For models with LCD : backlight control (0~255)
  M5.Display.setBrightness(96);

  M5.Display.startWrite();
  M5.Display.setFont(&lgfx::v1::fonts::DejaVu18);
  M5.Display.println("Receivers:");

  Wire.begin(0, 26);

//   M5.Lcd.setSwapBytes(false);
//   Disbuff.createSprite(80, 160);
//   Disbuff.setSwapBytes(true);
//   Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));
//   Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
//   Disbuff.pushSprite(0, 0);

  uint8_t res = I2CRead8bit(0x32);
  Serial.printf("Res0 = %02X \r\n", res);

//   M5.update();

  joystick.setTelemetryCallbacks(new MyTelemetryCallbacks());
  jm = joystick.newJoystickMsg();
  joystick.init();

//   Disbuff.setTextSize(1);
//   Disbuff.setTextColor(WHTE);
}

void loop() {
  // auto power off if receiver is not connected
  if (!receiverConnected && suspendCount++ > 1000) {
    Serial.println("not receiver detected. Turn off..");
    Serial.println("Releasing receivers:");
    joystick.printReceivers();
    delay(1000);
    M5.Power.powerOff();
  }

//   if (M5.BtnA.read() == 1) {
//     if (btnPowerOffcount++ > 10) {
//       std::vector<uint32_t> receivers = joystick.getReceivers();
//       for (uint32_t recv : receivers) {
//         Serial.println(recv);
//       }
//       delay(1000);
//       M5.Axp.PowerOff();
//     }
//   }

  for (int i = 0; i < 4; i++) {
    AngleBuff[i] = I2CRead16bit(0x50 + i * 2);
  }

  uint8_t ax = map(AngleBuff[0], 0, 4000, 0, 200);
  uint8_t ay = map(AngleBuff[1], 0, 4000, 0, 200);
  uint8_t az = map(AngleBuff[2], 0, 4000, 0, 200);
  uint8_t ck = 0x00;

  if ((ax > 110) || (ax < 110) ||
      (ay > 110) || (ay < 110) ||
      (az > 110) || (az < 110)) {
    ck = 0x01;
  }

  jm.ay = ay;
  jm.ax = ax;
  jm.az = az;
  jm.ck = ck;

  if(jm.ck != 0x00) joystick.sendJoystickMsg(jm);
  updateDisplay(ax, ay, az);
}
