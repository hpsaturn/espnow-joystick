[![PlatformIO](https://github.com/hpsaturn/espnow-joystick/workflows/PlatformIO/badge.svg)](https://github.com/hpsaturn/espnow-joystick/actions/) ![ViewCount](https://views.whatilearened.today/views/github/hpsaturn/espnow-joystick.svg) 

# ESPNow Joystick

Abstraction of ESP-Now and Protocol Buffers to have improved joystick for any kind of hardware, with a simple callback implementations.

<table>
  <tr>
    <td>
      Don't forget to star ⭐ this repository
    </td>
  </tr>
</table>

## Features

- [x] Full ESP-Now abstraction (broadcast, P2P, and auto)
- [x] Nanopb protos implementation (improve payload and channel)
- [x] Telemetry events and Joystick parameters callbacks
- [x] Full joystick and receiver examples with a M5Stack Joytstick
- [x] P2P option for handling single device
- [x] Generic ESP32 and ESP8266 support
- [x] Auto receivers detection. (Mac Address Vector)
- [x] Dynamic joystick sample (select one of multi target detected)
- [x] Added on the Arduino Library Manager for easy installation
- [ ] Custom proto definitions

[Demo video1](https://www.youtube.com/watch?v=pZbMmkq8tUw)  
[Demo video2](https://youtu.be/FcnYnp4PD0Y?si=3FyaXl4QsYyuY-1y)

## Joystick Implementation

You only need to pass the Joystick message and fill any of the possibilities, for example:

```cpp
EspNowJoystick joystick;
JoystickMessage jm;
bool receiverConnected;

// callback to telemetry values (not mandatory)
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
    // any implementation, SPI, i2c, analog switchs
    uint8_t ax = map(random(0,100), 0, 100, 0, 200);  
    uint8_t ay = map(random(0,100), 0, 100, 0, 200);
    uint8_t az = map(random(0,100), 0, 100, 0, 200);

    // You are able to fill more variables. See the #proto-definition
    jm.ay = ay;
    jm.ax = ax;
    jm.az = az;

    joystick.sendJoystickMsg(jm);
}
```

## Receiver Implementation

You only need to pass the Telemetry message if you want, and implement the joystick callbacks to receive the parameters for your receiver, it is not mandatory:

```cpp
EspNowJoystick joystick;
TelemetryMessage tm;

// your implementation
void setSpeed(int32_t ax, int32_t ay, int32_t az) {
    Serial.printf("ax:%04i ay:%04i az:%04i\r\n", ax, ay, az);
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
        setSpeed(jm.ax, jm.ay, jm.az);
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

For send to specific device, you only need to define the address of the device, for example:

```cpp
const uint8_t device1[6] = {0x3C, 0x61, 0x05, 0x0c, 0x93, 0xb8};

void loop() {
    ...
    joystick.sendJoystickMsg(jm,device1); 
}
```

You able to catch the macaddress enabling the debug mode with `joystick.init(true)` on the setup.  

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

## Examples

**PlatformIO:**  

For build and upload the examples here, with [PlatformIO](https://platformio.org/) is more easy, for example from command line you able to upload the M5Stack Joystick firmware, for M5StickC and M5StickCPlus with a simple:

```bash
cd examples/m5unified-joystick/
pio run --target upload
```

or  

```bash
cd examples/mini-tank/
pio run --target upload
```

Also you can make a [DC motors tank](https://github.com/hpsaturn/espnow-joystick/blob/master/examples/robot) or a servo motors [Mini-tank](https://github.com/hpsaturn/mini-tank#readme) or an [Arkanoid Game](https://github.com/hpsaturn/espnow-joystick/blob/master/examples/arkanoid) receiver. See the examples directory.

**Arduino IDE:**

For `Arduino IDE` is a little bit more complicated because the Arduino IDE dependencies resolver is very bad, but you only need first download and install the [Nanopb library](https://github.com/nanopb/nanopb/releases/tag/nanopb-0.4.8) using the `Include Library` section via zip file, and then with the **Library Manager** find **ESPNowJoystick** like this:

![screenshot20231121_002715](https://github.com/hpsaturn/espnow-joystick/assets/423856/b655efdb-9dfb-411d-b265-a7398a1b9065)  

or download and install ESPNowJoystick from the [releases section](https://github.com/hpsaturn/espnow-joystick/releases).

**Troubleshooting**:

```cpp
EspNowJoystick.hpp:31:10: fatal error: pb_decode.h: No such file or directory
```

or

![screenshot20231120_235544](https://github.com/hpsaturn/espnow-joystick/assets/423856/b6523921-efe3-40de-8b33-f0b730c9113a)

Don't forget install first the [Nanopb library](https://github.com/nanopb/nanopb/releases/tag/nanopb-0.4.8)

## Proto Definition

Only for information, **you don't need to do anything here**. This is the current payload [proto definition](https://github.com/hpsaturn/espnow-joystick/blob/master/src/comm.proto) that use the library for default. Thanks to this, you only need to set the messages like the examples, i.e: `jm.bA = 1;` in your code.

In the next version, the idea is maybe pass a custom proto to improve the size or extend the current protocol. The current version only consumes 25 bytes on the Joystick message.

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
d318618 rev083v0.0.9 Fixed telemetry issues on multiple targets
24e74bd added basic filter for telemetry when exist a P2P target
1f64c44 new error callback documentation and added p2p section
84fbe0a fixed issue with view badge count
fdede0d added basic workflow and Github CI stuff for testing
572dca3 P2P option for handling single device
3e84e1c p2p methods working and fixed issue with error callback
```

### v0.0.7r072

[![screenshot20220215_010838small](https://user-images.githubusercontent.com/423856/154026452-cd96ca60-f828-4463-8909-a6da1e114667.jpg)](https://www.youtube.com/watch?v=pZbMmkq8tUw)

```bash
e590ace ESP-Now abstraction with broadcast support and callbacks  
ce0db02 robot side Nanopb protos implementation
11180e9 Basic debug mode  
a12cb41 Full example included with M5Stack Joystick and Arkanoid
```
