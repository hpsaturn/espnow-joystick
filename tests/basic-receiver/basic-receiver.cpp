#include <Arduino.h>
#include <EspNowJoystick.hpp>

EspNowJoystick joystick;
TelemetryMessage tm;

// your implementation
void setSpeed(int32_t ax, int32_t ay, int32_t az) {
  Serial.printf("ax:%05i ay:%05i az:%05i\r\n", ax, ay, az);
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
    void onJoystickMsg(JoystickMessage jm){
        if (jm.ck == 0x01) {
            setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        } else {
            setSpeed(0, 0, 0);
        }
        sendHeartbeat();
    };
    void onError(const char *msg) {
        setSpeed(0, 0, 0);
        Serial.printf("Error %s\r\n",msg);
    };
};

void setup() {
    Serial.begin(115200);
    joystick.setJoystickCallbacks(new MyJoystickCallback());
    tm = joystick.newTelemetryMsg();
    joystick.init();
}

void loop() {}