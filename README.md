[![PlatformIO](https://github.com/hpsaturn/espnow-joystick/workflows/PlatformIO/badge.svg)](https://github.com/hpsaturn/espnow-joystick/actions/) ![ViewCount](https://views.whatilearened.today/views/github/hpsaturn/espnow-joystick.svg) 

# ESPNow Joystick 

Abstraccion of ESP-Now and Protocol Buffers to have improved joystick for any kind of hardware, with a simple callback implementations.

## TODO

- [x] ESP-Now abstraction (broadcast only for now)
- [x] Nanopb protos implementation (improve payload and channel)
- [x] Telemetry and Joystick callbacks
- [x] Full joystick and receiver example on M5Stack Joytstick
- [x] Basic examples with differente hardware
- [x] P2P option for handling single device
- [ ] Custom proto definitions
- [ ] Limit to only specific receiver (now the joystick handled many at the same time :D)

[Demo video](https://www.youtube.com/watch?v=pZbMmkq8tUw)

## Joystick Implementation

You only need pass the Joystick message and fill any of the possibilities, for example:

```cpp
EspNowJoystick joystick;
JoystickMessage jm;
bool receiverConnected;

// callback to telemetries values (not mandatory)
class MyTelemetryCallbacks : public EspNowTelemetryCallbacks{
    void onTelemetryMsg(TelemetryMessage tm){
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

    jm.ay = ay;  // You can fill more variables. See the comm.proto definitions
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

## Proto Definitions

Only for information, **you don't need to do anything here**. This the current payload protocol that use the library for default. But you only need set the messages like the examples, i.e: `jm.bA = 1;` in your code.

```cpp
syntax = "proto2";

message JoystickMessage {
    required sint32 ax = 1;  // left stick x position
    required sint32 ay = 2;  // left stick y position
    required sint32 az = 3;  // right stick z position
    required sint32 aw = 4;  // right stick w position
    required int32 bA = 5;   // buttonA data
    required int32 bB = 6;   // buttonB data
    required int32 bX = 7;   // buttonX data
    required int32 bY = 8;   // buttonY data
    required int32 bL = 9;   // buttonL data
    required int32 bR = 10;  // buttonR data
    required int32 bU = 11;  // buttonUp data
    required int32 bD = 12;  // buttonDown data
    required int32 ck = 13;  // check data
}

message TelemetryMessage {
    required uint32 btl = 1; // battery level
    required float  btv = 2; // battery voltage
    required sint32 x = 3;   // x position
    required sint32 y = 4;   // y position
    required sint32 z = 5;   // z position
    required bool e1 = 6;    // event1 data
    required bool e2 = 7;    // event2 data
    required bool e3 = 8;    // event3 data
    required float t1 = 9;   // variable1 data
    required float t2 = 10;  // variable2 data
    required uint32 ck = 11; // check data
}
```
In the next version the idea its maybe pass a custom proto for improve the size or extend the current protocol. The current version only consume 25 bytes on the Joystick message.

## Changelog

### r072v0.0.7

[![screenshot20220215_010838small](https://user-images.githubusercontent.com/423856/154026452-cd96ca60-f828-4463-8909-a6da1e114667.jpg)](https://www.youtube.com/watch?v=pZbMmkq8tUw)

- [x] Full ESP-Now abstraction with broadcast support and easy callbacks
- [x] Joystick and telemetry messages implemented with Nanopb protos
- [x] Basi debug mode
- [x] Full example included with M5Stack Joystick and Arkanoid game
- [x] Full example of basic two motor robot (receiver)
- [x] Basic examples   


