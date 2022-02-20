#include <Arduino.h>
#include <EspNowJoystick.hpp>

EspNowJoystick joystick;
JoystickMessage jm;
bool receiverConnected;

const uint8_t device1[6] = {0x3C, 0x61, 0x05, 0x0c, 0x93, 0xb8};

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

    // joystick.sendJoystickMsg(jm);       // send to all devices (broadcast)
    joystick.sendJoystickMsg(jm,device1);  // send to specific device
}