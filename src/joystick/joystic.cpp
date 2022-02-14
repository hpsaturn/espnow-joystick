#include <M5StickC.h>
#include <EspNowJoystick.hpp>
#include <ConfigApp.hpp>
#include <pb_decode.h>
#include <pb_encode.h>

#include "comm.pb.h"

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
extern const unsigned char connect_on[800];
extern const unsigned char connect_off[800];

uint64_t realTime[4], time_count = 0;
bool k_ready = false;
uint32_t key_count = 0;
bool ledOn = false;

uint32_t count_bn_a = 0, choose = 0;
String ssidname;

uint8_t adc_value[5] = {0};
uint16_t AngleBuff[4];
uint32_t count = 0;

EspNowJoystick joystick;
JoystickMessage jm = JoystickMessage_init_zero;

uint8_t I2CRead8bit(uint8_t Addr) {
    Wire.beginTransmission(0x38);
    Wire.write(Addr);
    Wire.endTransmission();
    Wire.requestFrom(0x38, 1);
    return Wire.read();
}

uint16_t I2CRead16bit(uint8_t Addr) {
    uint16_t ReData = 0;
    Wire.beginTransmission(0x38);
    Wire.write(Addr);
    Wire.endTransmission();
    Wire.requestFrom(0x38, 2);
    for (int i = 0; i < 2; i++) {
        ReData <<= 8;
        ReData |= Wire.read();
    }
    return ReData;
}

class MyTelemetryCallbacks : public EspNowTelemetryCallbacks{
    void onTelemetryMsg(TelemetryMessage tm){

    };
    void onError(){

    };
};

void setup() {
    M5.begin();
    Wire.begin(0, 26, 10000);
    cfg.init();

    M5.Lcd.setRotation(4);
    M5.Lcd.setSwapBytes(false);
    Disbuff.createSprite(80, 160);
    Disbuff.setSwapBytes(true);
    Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));
    Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_off);
    Disbuff.pushSprite(0, 0);

    uint8_t res = I2CRead8bit(0x32);
    Serial.printf("Res0 = %02X \r\n", res);

    M5.update();

    // lastDevice = cfg.loadString(PREF_LAST_DEVICE);

    joystick.setTelemetryCallbacks(new MyTelemetryCallbacks());
    while(!joystick.init());

    Disbuff.setTextSize(1);
    Disbuff.setTextColor(WHITE);
    Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));
    Disbuff.fillRect(0, 20, 80, 140, BLACK);
    Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
    Disbuff.pushSprite(0, 0);
    Disbuff.setTextColor(WHITE);
    count=0;
}

void drawValues(uint8_t ax, uint8_t ay, uint8_t az) {
    Disbuff.fillRect(0, 30, 80, 130, BLACK);
    Disbuff.setCursor(10, 30);
    Disbuff.printf("AX: %04d", ax);
    Disbuff.setCursor(10, 45);
    Disbuff.printf("AY: %04d", ay);
    Disbuff.setCursor(10, 60);
    Disbuff.printf("AZ: %04d", az);
    Disbuff.setCursor(10, 85);
    Disbuff.printf("V: %.2fv", M5.Axp.GetBatVoltage());
    Disbuff.setCursor(10, 95);
    Disbuff.printf("I: %.1fma", M5.Axp.GetBatCurrent());
}


void updateDisplay(uint8_t ax, uint8_t ay, uint8_t az) {
    static uint_least32_t guiTimeStamp = 0;
    if (millis() - guiTimeStamp > 80) {
        guiTimeStamp = millis();
        Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
        Disbuff.pushSprite(0, 0);
        drawValues(ax, ay, az);
        M5.update();
    }
}

void loop() {
    if (M5.BtnA.read() == 1) {
        if (count++ > 10) M5.Axp.PowerOff();
    }

    for (int i = 0; i < 4; i++) {
        AngleBuff[i] = I2CRead16bit(0x50 + i * 2);
    }

    uint8_t ax = map(AngleBuff[0], 0, 4000, 0, 200);
    uint8_t ay = map(AngleBuff[1], 0, 4000, 0, 200);
    uint8_t az = map(AngleBuff[2], 0, 4000, 0, 200);
    uint8_t ck = 0x00;

    if ((ax > 110) || (ax < 90) ||
        (ay > 110) || (ay < 90) ||
        (az > 110) || (az < 90)) {
        ck = 0x01;
    }
    jm.ax = ax;
    jm.ay = ay;
    jm.az = az;
    jm.ba = 1;
    jm.ck = ck;
    joystick.sendJoystickMsg(jm);
    updateDisplay(ax, ay, az);
}