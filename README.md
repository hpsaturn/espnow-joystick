# AIRobot

ESP32 robot platform, the idea is join it with a [MaxiPy RISC-V](https://maixpy.sipeed.com/en/) camera. For now the current development works with a M5StickC Joystick hat that controlling the robot via UDP using protobuf (nanopb). In the robot it use a simple ESP32 board.

## TODO

- [x] Platformio project (two sources)
- [x] UDP channel settings in preferences lib
- [x] nanopb (protocol buffers implementation) for joystick messages
- [x] separated OTA (joystick and robot)
- [ ] MQTT channel ?
- [ ] SPI connection to MaxiPy nano camera
- [ ] Push AI models via proto
- [ ] Auto navegation
- [ ] Seek and destroy objects

## Firmware

You can build it with Arduino IDE renaming the main files to .ino, but it is more easy if you use PlatformIO, with a simple command you upload both, Joystick and Robot.

```bash
pio run --target upload
```

Please check the right USB ports on `platformio.ini` file.

After the first upload, you could comment the robot and joystick blocks in platformio.ini and uncomment the OTA blocks for upload via WiFi without USB.

## DIY Robot

For the instructions and more details [here](https://www.thingiverse.com/thing:4705776).