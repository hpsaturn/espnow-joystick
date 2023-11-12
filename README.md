[![PlatformIO](https://github.com/hpsaturn/espnow-joystick/workflows/PlatformIO/badge.svg)](https://github.com/hpsaturn/espnow-joystick/actions/) ![ViewCount](https://views.whatilearened.today/views/github/hpsaturn/espnow-joystick.svg) 

# ESPNow Joystick

Abstraccion of ESP-Now and Protocol Buffers to have improved joystick for any kind of hardware, with a simple callback implementation.

<table>
  <tr>
    <td>
      Don't forget to star ⭐ this repository
    </td>
  </tr>
</table>

## TODO

- [x] ESP-Now abstraction (broadcast only for now)
- [x] Nanopb protos implementation (improve payload and channel)
- [x] Telemetry and Joystick callbacks
- [x] Full joystick and receiver example on M5Stack Joytstick
- [x] Basic examples with differente hardware
- [x] P2P option for handling single device
- [x] ESP8266 support
- [x] auto receivers detection. (mac address vector)
- [x] Dynamic joystick (select one of multi target detected)
- [ ] Custom proto definitions
- [ ] Limit to only specific receiver (now the joystick handled many at the same time :D)

[Demo video1](https://www.youtube.com/watch?v=pZbMmkq8tUw)
[Demo video2](https://youtu.be/FcnYnp4PD0Y?si=3FyaXl4QsYyuY-1y)

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
```

## Receiver Implementation

You only need pass the Telemetry message if you want, it is not mandatory, and implement the joystick callbacks to receive the parameters for your receiver:

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
    void onError(const char *msg){
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

## P2P Implementation

For send to specific device you only need specificate the address of the device, for example:

```cpp
const uint8_t device1[6] = {0x3C, 0x61, 0x05, 0x0c, 0x93, 0xb8};

void loop() {
    ...
    joystick.sendJoystickMsg(jm,device1); 
}
```

**Note:** in the last version, you don't need specify the mac address of the receiver target. The library can store the mac address detected joystick around, and you are able select it from a simple vector. Please see bellow:

## Multiple receivers handling

The last version is able to generate a device Id vector of the devices detected. With it is possible retrieve its mac address of each one. For example:

```cpp
std::vector<uint32_t> receivers = joystick.getReceivers();
uint32_t deviceId = receivers[0];
const uint8_t macAddress = joystick.getReceiverMacAddr(deviceId);
```

More info in the `m5unified-joystick` example. A little demo of this feature here:

[![ESPNow Joystick multiple receiver demo](https://github-production-user-asset-6210df.s3.amazonaws.com/423856/282314318-c4a59c87-6d21-4183-ac82-f89c8e1bc470.jpg)](https://youtu.be/FcnYnp4PD0Y)

You can catch the macaddress enabling the debug mode with `joystick.init(true)` on the setup.

## Proto Definitions

Only for information, **you don't need to do anything here**. This is the current payload protocol that use the library for default. But you only need set the messages like the examples, i.e: `jm.bA = 1;` in your code.

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

### v0.1.0r084

```bash
a1a1403 M5Unified version with receiver selection feature (BtnA)
27b222d added env for multi receiver handling demo and sample
def35a4 initial implementation with M5Unified library.
8b68b1b auto detection of receivers and methods to handled it
4444222 new esp8266 support. Improved SDK optimizations
c6074e8 fixed issue with espressif tools
88088f7 added macaddress catch tip
```

### v0.0.9rev083

```bash
d318618 rev083v0.0.9 Fixed telemetry issue when are multiple targets
24e74bd added basic filter for telemetry when exist a P2P target
1f64c44 fixed issues with new error callback documentation and added p2p section
84fbe0a fixed issue with view badge count
fdede0d added basic workflow and Github CI stuff for testing
572dca3 P2P option for handling single device
3e84e1c p2p methods working and fixed issue with error callback
```

### v0.0.7r072

[![screenshot20220215_010838small](https://user-images.githubusercontent.com/423856/154026452-cd96ca60-f828-4463-8909-a6da1e114667.jpg)](https://www.youtube.com/watch?v=pZbMmkq8tUw)

```bash
e590ace ESP-Now abstraction with broadcast support and callbacks  
Joystick and telemetry messages implemented with Nanopb protos
B11180e9 asic debug mode  
a12cb41 Full example included with M5Stack Joystick and Arkanoid game
```
