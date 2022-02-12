#include <M5StickC.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ConfigApp.hpp>
#include <pb_decode.h>
#include <pb_encode.h>

#include "comm.pb.h"

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
extern const unsigned char connect_on[800];
extern const unsigned char connect_off[800];

#define SYSNUM 3

uint64_t realTime[4], time_count = 0;
bool k_ready = false;
uint32_t key_count = 0;
bool ledOn = false;

uint32_t count_bn_a = 0, choose = 0;
String ssidname;

uint8_t adc_value[5] = {0};
uint16_t AngleBuff[4];
uint32_t count = 0;

uint8_t buffer[128];
size_t message_length;
bool status;

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength) {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
    // only allow a maximum of 250 characters in the message + a null terminating byte
    char buffer[ESP_NOW_MAX_DATA_LEN + 1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    strncpy(buffer, (const char *)data, msgLen);
    // make sure we are null terminated
    buffer[msgLen] = 0;
    // format the mac address
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    // debug log the message to the serial port
    Serial.printf("Received message from: %s - %s\n", macStr, buffer);
    // what are our instructions
    if (strcmp("on", buffer) == 0) {
        ledOn = true;
    } else {
        ledOn = false;
    }
    digitalWrite(2, ledOn);
}

// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    // Serial.print("Last Packet Sent to: ");
    // Serial.println(macStr);
    // Serial.print("Last Packet Send Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/**
 * @brief sendMessage via protobuf (nanopb)
 * @param ax stick1 x value
 * @param ay stick1 y value
 * @param az stick2 x value
 * @param ck check value
 */
bool sendMessage(uint8_t ax, uint8_t ay, uint8_t az, uint8_t ck) {
    JoystickMessage jm = JoystickMessage_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    /* Fill the proto values (joystick angle)*/
    jm.ax = ax;
    jm.ay = ay;
    jm.az = az;
    jm.ck = ck;

    status = pb_encode(&stream, JoystickMessage_fields, &jm);
    message_length = stream.bytes_written;

    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }

    uint8_t sendBuff[message_length + 4];

    sendBuff[0] = 0xAA;  // UDP header?
    sendBuff[1] = 0x55;
    sendBuff[2] = SYSNUM;  // UDP type?

    for (int i = 0; i < message_length; i++) {
        sendBuff[i + 3] = buffer[i];  // proto message
    }

    sendBuff[message_length + 3] = 0xee;  // UDP end?

    // this will broadcast a message to everyone in range
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    if (!esp_now_is_peer_exist(broadcastAddress)) {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(broadcastAddress, sendBuff, message_length + 4);
    // and this will send a message to a specific device
    /*uint8_t peerAddress[] = {0x3C, 0x71, 0xBF, 0x47, 0xA5, 0xC0};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, peerAddress, 6);
    if (!esp_now_is_peer_exist(peerAddress))
    {
      esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());*/
    if (result == ESP_OK) {
        // Serial.println("Broadcast message success");
        return true;
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
    } else {
        Serial.println("Unknown error");
    }
    return false;
}

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

    Disbuff.setTextSize(1);
    Disbuff.setTextColor(WHITE);
    Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));

    WiFi.mode(WIFI_STA);
    // startup ESP Now
    Serial.println("ESPNow Init");
    Serial.println(WiFi.macAddress());
    // shut down wifi
    WiFi.disconnect();

    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
        esp_now_register_recv_cb(receiveCallback);
        esp_now_register_send_cb(sentCallback);
    } else {
        Serial.println("ESPNow Init Failed");
        delay(3000);
        ESP.restart();
    }

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

void loop() {
    if (M5.BtnA.read() == 1) {
        if (count++ > 10) M5.Axp.PowerOff();
    }

    for (int i = 0; i < 4; i++) {
        AngleBuff[i] = I2CRead16bit(0x50 + i * 2);
    }

    Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
    Disbuff.pushSprite(0, 0);

    uint8_t ax = map(AngleBuff[0], 0, 4000, 0, 200);
    uint8_t ay = map(AngleBuff[1], 0, 4000, 0, 200);
    uint8_t az = map(AngleBuff[2], 0, 4000, 0, 200);
    uint8_t ck = 0x00;

    if ((ax > 110) || (ax < 90) ||
        (ay > 110) || (ay < 90) ||
        (az > 110) || (az < 90)) {
        ck = 0x01;
    }

    drawValues(ax, ay, az);
    sendMessage(ax, ay, az, ck);

    M5.update();

    delay(10);
}