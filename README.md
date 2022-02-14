
# ESPNow Joystick 

Abstraccion of ESPNow and Protocol Buffers to have improved joystick for any kind of joysitck hardware, with a simple callback implementations.

# Joystick Implementation

You only need pass the Joystick message and fill any of the possibilities, for example:

```cpp
EspNowJoystick joystick;
JoystickMessage jm;

// callback to telemetries values
class MyTelemetryCallbacks : public EspNowTelemetryCallbacks{
    void onTelemetryMsg(TelemetryMessage tm){
        Serial.printf("TelemetryMsg: %0.2fV %d%%\n", tm.btv, tm.btl);
        receiverBattVolt = tm.btv;
        receiverBattLevel = tm.btl;
        receiverConnected = tm.e1;
    };
    void onError(){
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

    jm.ay = ay;
    jm.ax = ax;
    jm.az = az;

    joystick.sendJoystickMsg(jm);
}

```

## Receiver Implementation

You only need pass the Telemetry message if you want, it is not mandatory, and implement the joystick callbacs:

```cpp
EspNowJoystick joystick;
TelemetryMessage tm;

void sendHeartbeat() {
    static uint_least32_t timeStamp = 0;
    if (millis() - timeStamp > 500) {
        timeStamp = millis();
        tm.e1 = true;
        joystick.sendTelemetryMsg(tm);
    }
}

class MyJoystickCallback : public EspNowJoystickCallbacks {
    void onJoystickMsg(JoystickMessage jm){
        if (jm.ck == 0x01) {
            setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        } else {
            setSpeed(0, 0, 0);
        }
        sendHeartbeat();
    };
    void onError(){
        setSpeed(0, 0, 0);
        Serial.println("Error");
    };
};

void setup() {
    joystick.setJoystickCallbacks(new MyJoystickCallback());
    tm = joystick.newTelemetryMsg();
    joystick.init();
}

void loop() {}
```

