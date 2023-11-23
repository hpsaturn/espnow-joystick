#include <Arduino.h>

#include <EspNowJoystick.hpp>

EspNowJoystick joystick;
TelemetryMessage tm;

// your implementation
void setSpeed(int32_t ax, int32_t ay, int32_t az) {
  Serial.printf("command: ax:%04i ay:%04i az:%04i\r\n", ax, ay, az);
}

// optional: Send telemetry or status of the receiver
void sendHeartbeat() {
  static uint_least32_t timeStamp = 0;
  if (millis() - timeStamp > 500) {
    timeStamp = millis();
    tm.e1 = true;
    joystick.sendTelemetryMsg(tm);
  }
}

// Joystick data
class MyJoystickCallback : public EspNowJoystickCallbacks {
  void onJoystickMsg(JoystickMessage jm) {
    setSpeed(jm.ax, jm.ay, jm.az);
    sendHeartbeat();
  };
  void onError(const char *msg) {
    setSpeed(0, 0, 0);
    Serial.printf("Error %s\r\n", msg);
  };
};

void setup() {
  Serial.begin(115200);
  joystick.setJoystickCallbacks(new MyJoystickCallback());
  tm = joystick.newTelemetryMsg();
  joystick.init();
}

void loop() {}