#include <Arduino.h>
#include <EspNowJoystick.hpp>

EspNowJoystick joystick;
JoystickMessage jm;

// callback to telemetries values (not mandatory)
class MyTelemetryCallbacks : public EspNowTelemetryCallbacks {
  void onTelemetryMsg(TelemetryMessage tm) {
    bool boolFromRecv = tm.e1;
    Serial.printf("Receiver msg received, bool -> %s\r\n", boolFromRecv ? "true" : "false");
  };
  void onError(const char *msg){};
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  joystick.setTelemetryCallbacks(new MyTelemetryCallbacks());
  jm = joystick.newJoystickMsg();
  joystick.init();
}

void loop() {
  // any implementation, SPI, i2c, analog switchs
  uint8_t ax = map(random(0, 100), 0, 100, 0, 200);  
  uint8_t ay = map(random(0, 100), 0, 100, 0, 200);
  uint8_t az = map(random(0, 100), 0, 100, 0, 200);

  // You can fill more variables. See the comm.proto definitions
  jm.ax = ax;
  jm.ay = ay;  
  jm.az = az;

  // Serial.printf("sending ax:%04i ay:%04i az:%04i\r\n", ax, ay, az);
  joystick.sendJoystickMsg(jm);

  // ONLY for demo (you can remove it);
  delay(1000); 
}