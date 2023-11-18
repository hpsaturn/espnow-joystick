#include <Arduino.h>
#include <EspNowJoystick.hpp>

EspNowJoystick joystick;
JoystickMessage jm;
bool receiverConnected;

// callback to telemetries values (not mandatory)
class MyTelemetryCallbacks : public EspNowTelemetryCallbacks{
    void onTelemetryMsg(TelemetryMessage tm){
        Serial.println("Telemetry msg received");
        receiverConnected = tm.e1;
    };
    void onError(const char *msg){
    };
};

void setup() {
    joystick.setTelemetryCallbacks(new MyTelemetryCallbacks());
    jm = joystick.newJoystickMsg();
    joystick.init();
}

void loop() {
    uint8_t ax = map(random(0,100), 0, 100, 0, 200);  // any implementation, SPI, i2c, analog switchs
    uint8_t ay = map(random(0,100), 0, 100, 0, 200);
    uint8_t az = map(random(0,100), 0, 100, 0, 200);

    jm.ay = ay;  // You can fill more variables. See the comm.proto definitions
    jm.ax = ax;
    jm.az = az;

    joystick.sendJoystickMsg(jm);
}